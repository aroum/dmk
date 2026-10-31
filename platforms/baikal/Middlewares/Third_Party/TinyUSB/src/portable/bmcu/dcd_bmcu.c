/**
 * *****************************************************************************
 *  @file       dcd_bmcu.c
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB Device driver source file
 *  @version    2.4.0
 *  @date       2026-10-06
 * *****************************************************************************
 *  @copyright Copyright (c) 2026 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#include "dcd_bmcu.h"
#include "tusb_option.h"

#if CFG_TUD_ENABLED && CFG_TUSB_MCU == OPT_MCU_BMCU

#include "../../device/dcd.h"

#include "../../../hw/bsp/bmcu/board.h"

#define EP0_FIFO_SIZE               64U     /*!< The Endpoint 0 FIFO has a fixed size (64 bytes) and a fixed location (start address 0) */

#if defined (BE_U1000) && (USBD_USE_DMA)
#define DMA_CH_MAX              (5U)        /*!< The maximum number of DMA channels */
#define DMA_CH_NUM_MASK         (0x1FU)     /*!< DMA channel number mask */
#define DMA_CH_IN_USE_FLAG      (0x80U)     /*!< DMA channel is used for the selected channel flag */
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */

/** @brief Endpoint 0 state definitions */
typedef enum {
    EP0_STATE_IDLE = 0,             /*!< Idle state, wait for SETUP transaction */
    EP0_STATE_DATA_IN,              /*!< Data IN state */
    EP0_STATE_DATA_OUT,             /*!< Data OUT state */
    EP0_STATE_STATUS_IN,            /*!< Status IN state */
    EP0_STATE_STATUS_OUT,           /*!< Status OUT state */
    EP0_STATE_STATUS_SET_ADDRESS    /*!< Status state, handle Set Address request (special case of Status IN) */
} ep0_state_t;

/**
 * @brief USB transfer control structure
 * @note For the purpose of this structure definition, the USB transfer means
 *       the data transfer according to USB specification. Depending on the size
 *       of the TinyUSB stack internal buffer allocated for the control endpoint,
 *       the USB transfer may be split into multiple TinyUSB transfers (xfer). In
 *       its turn, a single xfer may be transmitted in multiple data packets.
 */
typedef struct {
    uint8_t* xfer_buf;          /*!< Pointer to the data buffer allocated for xfer */
    uint16_t xfer_len;          /*!< The size of the data buffer allocated for xfer.
                                     For IN direction it defines the length of the data to be sent to host,
                                     for OUT direction it is the size of the buffer allocated for the data from host. */
    uint16_t xferred;           /*!< The number of bytes transferred in the current xfer */
    uint16_t transferred;       /*!< The number of bytes transferred in the data stage of the current USB transfer.
                                     The length of the USB transfer is not saved in this structure since it is contained in the cached control request. */
    uint16_t last_packet_size;  /*!< The size of the last data packet sent or received */
} xfer_ctrl_t;

/** @brief Endpoint control structure */
typedef struct {
    uint8_t     ep_addr;            /*!< The address of the endpoint (bEndpointAddress) */
    uint16_t    max_packet_size;    /*!< The maximum packet size endpoint is capable of sending or receiving (wMaxPacketSize) */
    xfer_ctrl_t xfer;               /*!< The transfer control structure */
#if defined (BE_U1000) && (USBD_USE_DMA)
    uint8_t     dma_ch;             /*!< DMA channel assigned to the endpoint */
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
} epn_ctrl_t;

/** @brief USB Device Class Driver control structure */
typedef struct
{
    uint8_t                dev_addr;            /*!< Device address */
    uint16_t               fifo_addr_top;       /*!< Current address in FIFO RAM buffer used for allocation */
    tusb_control_request_t control_request;     /*!< Control request */
    ep0_state_t            ep0_state;           /*!< Endpoint 0 state */
    epn_ctrl_t             ep[USBD_EP_MAX][2];  /*!< The endpoint control structure */
} dcd_ctrl_t;

static dcd_ctrl_t dcd;

/** @brief The macro to get the EP transfer control structure pointer */
#define EP_CTRL_BASE(epnum, dir)        &dcd.ep[epnum][dir]

/** @brief Configures OUT endpoint. */
static void epn_rx_configure(uint8_t epnum, uint16_t max_packet_size, uint16_t fifo_addr, uint8_t xfer_type);
/** @brief Configures IN endpoint. */
static void epn_tx_configure(uint8_t epnum, uint16_t max_packet_size, uint16_t fifo_addr, uint8_t xfer_type);

/** @brief Submits IN data packet. */
static void ep0_data_in(epn_ctrl_t * edpt);
/** @brief Handles the received SETUP packet. */
static void ep0_setup_packet_handler(void);

/** @brief Endpoint 0 interrupt handler */
static void ep0_int_handler(void);
/** @brief Endpoint 1 to 15 RX interrupt handler */
static void epn_rx_int_handler(uint8_t epnum);
/** @brief Endpoint 1 to 15 TX interrupt handler */
static void epn_tx_int_handler(uint8_t epnum);

/** @brief Writes the endpoint FIFO. */
static void tx_fifo_write(uint8_t epnum, uint8_t const * buffer, uint16_t count);
/** @brief Reads the endpoint FIFO. */
static uint16_t rx_fifo_read(uint8_t epnum, uint8_t * buffer, uint16_t max_bytes);

/**
 * @brief Initializes the USB peripheral in device mode and enables it.
 * @note This function should enable internal D+/D- pull-up for enumeration.
 * @param rhport The USB peripheral number.
 */
bool dcd_init(uint8_t rhport, const tusb_rhport_init_t* rh_init)
{
    (void)rh_init;

    /* Reset the USB controller. Software reset is not implemented in BMCU-U and
       BE-U1000 microcontrollers. It's included for future compatibility. */
    USB->CTRL.SOFT_RST |= (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST);
    while ((USB->CTRL.SOFT_RST & (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST)) != 0U);
    
    /* Reserve EP0 FIFO */
    dcd.fifo_addr_top = EP0_FIFO_SIZE;

    /* Disable endpoint interrupts */
    USB->COMMON.INTRRXE = 0x0U;
    USB->COMMON.INTRTXE = 0x0U;

    /* Enable Reset/Suspend/Resume interrupts */
    USB->COMMON.INTRUSBE = USB_INTRUSBE_RESETIE |
                           USB_INTRUSBE_RESUMEIE |
                           USB_INTRUSBE_SUSPIE;

    /* Configure USB interrupts in CLIC */
#ifndef CORE_BM310S14
    CLIC_ConfigIRQ(CLIC_USB_IRQn,                   /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   USB_INT_LEVEL,                   /* Level */
                   USB_INT_PRIORITY,                /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */

#if defined (BE_U1000) && (USBD_USE_DMA)
    CLIC_ConfigIRQ(CLIC_USB_DMA_IRQn,               /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   USB_DMA_INT_LEVEL,               /* Level */
                   USB_DMA_INT_PRIORITY,            /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */

#endif

    dcd_connect(rhport);

    return true;
}

/**
 * @brief Deinitializes the USB device.
 * @param rhport The roothub port number.
 */
bool dcd_deinit(uint8_t rhport)
{
    (void)rhport;

    /* Reset the USB controller. Software reset is not implemented in BMCU-U and
       BE-U1000 microcontrollers. It's included for future compatibility. */
    USB->CTRL.SOFT_RST |= (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST);
    while ((USB->CTRL.SOFT_RST & (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST)) != 0U);

    /* The USB will be disconnected upon reset.
       Due to lack of implementation it is included separately. */
    dcd_disconnect(rhport);

    /* Disable interrupts */
    USB->COMMON.INTRUSBE = 0x0U;
    USB->COMMON.INTRRXE = 0x0U;
    USB->COMMON.INTRTXE = 0x0U;

    /* All endpoint FIFOs will be flushed upon USB reset condition. */

    return true;
}

/**
 * @brief Enables the USB device interrupts.
 * @param rhport The USB peripheral number.
 */
void dcd_int_enable(uint8_t rhport)
{
    (void)rhport;

#ifndef CORE_BM310S14
    CLIC_EnableIRQ(CLIC_USB_IRQn);
#if defined (BE_U1000) && (USBD_USE_DMA)
    CLIC_EnableIRQ(CLIC_USB_DMA_IRQn);
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
#endif
}

/**
 * @brief Disables the USB device interrupts.
 * @param rhport The USB peripheral number.
 */
void dcd_int_disable(uint8_t rhport)
{
    (void)rhport;

#ifndef CORE_BM310S14
    CLIC_DisableIRQ(CLIC_USB_IRQn);
#if defined (BE_U1000) && (USBD_USE_DMA)
    CLIC_DisableIRQ(CLIC_USB_DMA_IRQn);
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
#endif
}

/**
 * @brief Handles Set Address request.
 * @param rhport The USB peripheral number.
 * @param dev_arr The device address.
 */
void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
    (void)rhport;

    /* dcd_set_address() is called in place of dcd_edpt_xfer()
       when SET_ADDRESS request is received */

    /* Store the address in USB device control structure */
    /* It will be written to register on control request completion */
    dcd.dev_addr = dev_addr;

    /* Switch to SET ADDRESS (STATUS IN) state */
    dcd.ep0_state = EP0_STATE_STATUS_SET_ADDRESS;

    /* Acknowledge SETUP transaction */
    USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_SVCRPR | USB_CSR0L_PERIPH_DATAEND);
}

/**
 * @brief Wake up host remotely when suspended.
 * @param rhport The USB peripheral number.
 */
void dcd_remote_wakeup(uint8_t rhport)
{
    (void)rhport;

    USB->COMMON.POWER |= USB_POWER_RESUME;

    /* In Peripheral mode the CPU should clear this bit after 10ms (maximum 15ms). */

#if CFG_TUSB_OS != OPT_OS_NONE
    osal_task_delay(10UL);
#else
    __delay_ms(10UL);
#endif

    USB->COMMON.POWER &= ~USB_POWER_RESUME;
}

/**
 * @brief Attaches USB device to USB bus.
 * @note This function should enable internal D+/D- pull-up.
 * @param rhport The USB peripheral number.
 */
void dcd_connect(uint8_t rhport)
{
    (void)rhport;

    if (TUD_OPT_HIGH_SPEED)
    {
        /* Negotiate for High-speed mode */
        USB->COMMON.POWER |= USB_POWER_HSEN;
    }
    else
    {
        /* Operate in Full-speed mode */
        USB->COMMON.POWER &= ~USB_POWER_HSEN;
    }

    /* Set PHY in normal state and enable D+/D- lines */
    USB->COMMON.POWER |= USB_POWER_SOFTCONN;
}

/**
 * @brief Detaches USB device from USB bus.
 * @note This function should disable internal D+/D- pull-up.
 * @param rhport The USB peripheral number.
 */
void dcd_disconnect(uint8_t rhport)
{
    (void)rhport;

    /* Set PHY in non-driving mode and disable D+/D- lines */
    USB->COMMON.POWER &= ~USB_POWER_SOFTCONN;
}

/**
 * @brief Enables/disables Start-of-frame interrupt.
 * @param rhport The USB peripheral number.
 * @param en Enable state.
 */
void dcd_sof_enable(uint8_t rhport, bool en)
{
    (void)rhport;

    if (en)
    {
        /* Enable SOF interrupt */
        USB->COMMON.INTRUSBE |= USB_INTRUSBE_SOFIE;
    }
    else
    {
        /* Disable SOF interrupt */
        USB->COMMON.INTRUSBE &= ~USB_INTRUSBE_SOFIE;
    }
}

/**
 * @brief Configures a non-control endpoint.
 * @param rhport The USB peripheral number.
 * @param desc_edpt The endpoint descriptor.
 * @return True if the endpoint has been configured successfully, otherwise false.
 */
bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const * desc_edpt)
{
    (void)rhport;

    bool retval = true;

    uint8_t epnum = tu_edpt_number(desc_edpt->bEndpointAddress);
    uint8_t dir = tu_edpt_dir(desc_edpt->bEndpointAddress);

    if ((epnum > 0U) && (epnum < USBD_EP_MAX))
    {
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, dir);

        edpt->ep_addr = desc_edpt->bEndpointAddress;
        edpt->max_packet_size = tu_edpt_packet_size(desc_edpt);
        
        if (dir == TUSB_DIR_OUT)
        {
            epn_rx_configure(epnum, edpt->max_packet_size, dcd.fifo_addr_top, desc_edpt->bmAttributes.xfer);

#if defined (BE_U1000) && (USBD_USE_DMA)
            uint8_t dma_ch = (epnum << 1) - 1U;
            if (dma_ch <= (DMA_CH_MAX - 1U))
            {
                /* Use DMA channel for the selected endpoint */
                edpt->dma_ch = DMA_CH_IN_USE_FLAG | dma_ch;
            }
            else
            {
                /* Do not use DMA channel for the selected endpoint */
                edpt->dma_ch = 0x0U;
            }
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        }
        else
        {
            epn_tx_configure(epnum, edpt->max_packet_size, dcd.fifo_addr_top, desc_edpt->bmAttributes.xfer);

#if defined (BE_U1000) && (USBD_USE_DMA)
            uint8_t dma_ch = (epnum << 1);
            if (dma_ch <= (DMA_CH_MAX - 1U))
            {
                /* Use DMA channel for the selected endpoint */
                edpt->dma_ch = DMA_CH_IN_USE_FLAG | dma_ch;
            }
            else
            {
                /* Do not use DMA channel for the selected endpoint */
                edpt->dma_ch = 0x0U;
            }
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        }

#if defined(USBD_DPKTBUF)
        dcd.fifo_addr_top += edpt->max_packet_size << 1;
#else
        dcd.fifo_addr_top += edpt->max_packet_size;
#endif /* defined(USBD_DPKTBUF) */
    }
    else
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief Closes a non-control endpoint.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 */
void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
{
    (void)rhport;

    uint8_t epnum = tu_edpt_number(ep_addr);

    if ((epnum > 0U) && (epnum < USBD_EP_MAX))
    {
        /* Disable endpoint interrupts */
        USB->COMMON.INTRRXE &= ~(1U << epnum);
        USB->COMMON.INTRTXE &= ~(1U << epnum);
    }
}

/**
 * @brief Closes all non-control endpoints.
 * @param rhport The USB peripheral number.
 */
void dcd_edpt_close_all(uint8_t rhport)
{
    (void)rhport;

    /* Disable endpoint interrupts except the control endpoint */
    USB->COMMON.INTRRXE = 0x0U;
    USB->COMMON.INTRTXE &= 0x1U;

    /* Reset EP FIFO pointer and reserve EP0 FIFO */
    dcd.fifo_addr_top = EP0_FIFO_SIZE;
}

/**
 * @brief Configures the USB data transfer.
 * @note For control transfer the function is invoked before Data and Status
 *       stages. For non-control endpoints it is invoked once per USB transfer.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 * @param buffer The data buffer.
 * @param total_bytes The buffer length.
 * @param is_isr Executed from ISR flag.
 * @return True if configured successfully, otherwise false.
 */
bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t * buffer, uint16_t total_bytes, bool is_isr)
{
    (void)rhport;
    (void)is_isr;

    /* Get endpoint control structure */
    uint8_t epnum = tu_edpt_number(ep_addr);
    uint8_t dir = tu_edpt_dir(ep_addr);

    /* The control endpoint uses two control structures depending on data stage.
       The directions of SETUP and DATA stages are the same and both are opposite
       to the direction of STATUS stage. */

    epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, dir);

    /* Configure data buffer and size. It is done once per xfer. If the buffer
       size is greater than the endpoint size, the data will be transferred in
       multiple packets. */
    edpt->xfer.xfer_buf = buffer;
    edpt->xfer.xfer_len = total_bytes;
    edpt->xfer.xferred = 0U;
    edpt->xfer.last_packet_size = 0U;

    if (epnum == 0U)
    {
        /* Endpoint 0 (Control transfer) */
        switch (dcd.ep0_state)
        {
            case EP0_STATE_IDLE:
            {
                /* Idle state, process the new SETUP events */
                edpt->xfer.transferred = 0U;

                if (dcd.control_request.bmRequestType_bit.direction == TUSB_DIR_OUT)
                {
                    if (dcd.control_request.wLength == 0U)
                    {
                        /* Zero data request */

                        /* Switch to STATUS IN state */
                        dcd.ep0_state = EP0_STATE_STATUS_IN;

                        /* Acknowledge SETUP transaction */
                        USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_SVCRPR | USB_CSR0L_PERIPH_DATAEND);
                    }
                    else
                    {
                        /* Data write request */

                        /* Switch to DATA OUT state */
                        dcd.ep0_state = EP0_STATE_DATA_OUT;

                        /* Ready to receive the first OUT data packet */
                        /* The incoming packets will be handled in ISR */

                        /* Acknowledge SETUP transaction */
                        USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;
                    }
                }
                else
                {
                    /* Data read request */

                    /* Switch to DATA IN state */
                    dcd.ep0_state = EP0_STATE_DATA_IN;

                    /* Acknowledge SETUP transaction */
                    USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;

                    /* Submit the first IN data packet */
                    ep0_data_in(edpt);
                }
            }
            break;

            case EP0_STATE_DATA_IN:
            {
                /* Data IN state */

                /* The fact that the driver reached here means that there will be
                   the second or even later stack xfer allocated in the same USB
                   transfer */

                /* Submit the next IN data packet */
                ep0_data_in(edpt);
            }
            break;

            case EP0_STATE_DATA_OUT:
            {
                /* Data OUT state */

                /* The previous xfer is complete at this point, there may be
                   another one scheduled */
                if ((edpt->xfer.xfer_buf != NULL) && (edpt->xfer.xfer_len > 0U))
                {
                    /* Expect more data packets in the new xfer */
                    USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;
                }
                else
                {
                    /* To the moment the DATA stage of the transfer is complete.
                       This function call prepares the STATUS stage of the transfer
                       in the USB stack. */

                    /* Switch to STATUS IN state */
                    dcd.ep0_state = EP0_STATE_STATUS_IN;

                    /* Acknowledge the last OUT data packet */
                    USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_SVCRPR | USB_CSR0L_PERIPH_DATAEND);

                    /* When the host moves to the STATUS stage of the request, another
                       Endpoint 0 interrupt will be generated to indicate that the request
                       has completed. */
                }
            }
            break;

            case EP0_STATE_STATUS_OUT:
            {
                /* Status OUT state */

                /* To the moment the DATA stage of the transfer is complete.
                   This function call prepares the STATUS stage of the transfer
                   in the USB stack. Meanwhile EP0 is switched to IDLE state
                   and ready to enqueue the new SETUP event. */

                /* Switch to IDLE state */
                dcd.ep0_state = EP0_STATE_IDLE;
            }
            break;

            default:
            {
                /* No action */
            }
            break;
        }
    }
    else
    {
        /* Endpoints 1 to 15 */
        if (dir == TUSB_DIR_OUT)
        {
            /* Enable the endpoint interrupt */
            USB->COMMON.INTRRXE |= (1U << epnum);

            /* Clear RxPktRdy flag. The data have already been copied
               in the interrupt routine. */
            USB->EPCSR[epnum].PERIPH.RXCSRL &= ~USB_RXCSRL_PERIPH_RXPKTRDY;
        }
        else
        {
            /* Sent the first data packet */
            edpt->xfer.last_packet_size = tu_min16(edpt->max_packet_size, edpt->xfer.xfer_len - edpt->xfer.xferred);

#if defined (BE_U1000) && (USBD_USE_DMA)
            if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
            {
                uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

                USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
                USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                               USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                               USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
            }
            else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
            {
                tx_fifo_write(epnum,
                              (uint8_t*)((uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred),
                              edpt->xfer.last_packet_size);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
            }
        }
    }

    return true;
}

/**
 * @brief Submits IN data packet.
 * @param edpt The pointer to the endpoint control structure.
 */
void ep0_data_in(epn_ctrl_t * edpt)
{
    edpt->xfer.last_packet_size = tu_min16(edpt->max_packet_size, dcd.control_request.wLength - edpt->xfer.transferred);
    edpt->xfer.last_packet_size = tu_min16(edpt->xfer.last_packet_size, edpt->xfer.xfer_len - edpt->xfer.xferred);

#if defined (BE_U1000) && (USBD_USE_DMA)
    if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
    {
        uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

        USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
        USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
        USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                       USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                       USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
    }
    else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
    {
        tx_fifo_write(0U, (uint8_t*)((uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred), edpt->xfer.last_packet_size);

        /* If the requested data length exceeds the EP0 buffer size, the USB stack
           will schedule two or more xfers for the request. In this case, the first
           one should not set DataEnd bit. Also, for this reason the request length 
           is checked here, not the allocated buffer size. */
        if (((edpt->xfer.transferred + edpt->xfer.last_packet_size) >= dcd.control_request.wLength) ||
            (edpt->xfer.last_packet_size < edpt->max_packet_size))
        {
            /* Switch to STATUS OUT state */
            dcd.ep0_state = EP0_STATE_STATUS_OUT;

            /* Send the last data packet */
            USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_TXPKTRDY | USB_CSR0L_PERIPH_DATAEND);

            /* When the last packet has been sent to the host, no more Endpoint 0
               interrupts will be generated in the Data stage. The host moves to
               the Status stage of the request, and another Endpoint 0 interrupt
               will be generated to indicate that the request has completed. */
        }
        else
        {
            /* Send the data packet, more data packets expected */
            USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_TXPKTRDY;

            /* When the packet has been sent to the host, another Endpoint 0 interrupt
               will be generated and the next data packet can be written to the FIFO */
        }
    }
}

/**
 * @brief Submits a transfer using FIFO.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 * @param ff The FIFO.
 * @param total_bytes The data length.
 * @param is_isr Executed from ISR flag.
 * @return True on success, otherwise false.
 */
bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes, bool is_isr)
{
    (void)rhport;
    (void)ep_addr;
    (void)ff;
    (void)total_bytes;
    (void)is_isr;

    return false;
}

/**
 * @brief Puts the endpoint in stall state.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 */
void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr)
{
    (void)rhport;

    uint8_t epnum = tu_edpt_number(ep_addr);
    uint8_t dir = tu_edpt_dir(ep_addr);
  
    if (epnum == 0U)
    {
        /* Protocol stall due to function problem in control transfer */
        /* It indicates that the request is not understood 
           and lasts until the next SETUP transaction */
        USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SENDSTALL;
    }
    else
    {
        /* Functional stall */
        /* It is associated with non-control endpoints 
           and set or cleared by the host */
        if (dir == TUSB_DIR_OUT)
        {
            USB->EPCSR[epnum].PERIPH.RXCSRL |= USB_RXCSRL_PERIPH_SENDSTALL;
        }
        else
        {
            USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_SENDSTALL;
        }
    }
}

/**
 * @brief Clears the endpoint stall state.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 */
void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
    (void)rhport;

    uint8_t epnum = tu_edpt_number(ep_addr);
    uint8_t dir = tu_edpt_dir(ep_addr);
  
    if (epnum > 0U)
    {
        /* Clear functional stall of a non-control endpoint */
        if (dir == TUSB_DIR_OUT)
        {
            USB->EPCSR[epnum].PERIPH.RXCSRL &= ~(USB_RXCSRL_PERIPH_SENTSTALL | USB_RXCSRL_PERIPH_SENDSTALL);
            USB->EPCSR[epnum].PERIPH.RXCSRL |= USB_RXCSRL_PERIPH_CLRDT;
        }
        else
        {
            USB->EPCSR[epnum].PERIPH.TXCSRL &= ~(USB_TXCSRL_PERIPH_SENTSTALL | USB_TXCSRL_PERIPH_SENDSTALL);
            USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_CLRDT;
        }
    }
}

/**
 * @brief Initializes an isochronous endpoint.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 * @param largest_packet_size The largest packet size. The actual packet size
 *                            will be set during dcd_edpt_iso_activate() call.
 * @return True if the endpoint has been initialized successfully, otherwise false.
 */
bool dcd_edpt_iso_alloc(uint8_t rhport, uint8_t ep_addr, uint16_t largest_packet_size)
{
    (void)rhport;

    bool retval = true;

    uint8_t epnum = tu_edpt_number(ep_addr);
    uint8_t dir = tu_edpt_dir(ep_addr);

    if ((epnum > 0U) && (epnum < USBD_EP_MAX))
    {
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, dir);

        edpt->ep_addr = ep_addr;

        /* Find the next power of two value that is higher than largest_packet_size */
        uint8_t pow = 0U;
        while (largest_packet_size > 0U)
        {
            largest_packet_size >>= 1;
            pow++;
        }
        largest_packet_size = 1U;
        while (pow > 0U)
        {
            largest_packet_size <<= 1;
            pow--;
        }

        edpt->max_packet_size = largest_packet_size;
        
        if (dir == TUSB_DIR_OUT)
        {
            epn_rx_configure(epnum, edpt->max_packet_size, dcd.fifo_addr_top, TUSB_XFER_ISOCHRONOUS);

#if defined (BE_U1000) && (USBD_USE_DMA)
            uint8_t dma_ch = (epnum << 1) - 1U;
            if (dma_ch <= (DMA_CH_MAX - 1U))
            {
                /* Use DMA channel for the selected endpoint */
                edpt->dma_ch = DMA_CH_IN_USE_FLAG | dma_ch;
            }
            else
            {
                /* Do not use DMA channel for the selected endpoint */
                edpt->dma_ch = 0x0U;
            }
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        }
        else
        {
            epn_tx_configure(epnum, edpt->max_packet_size, dcd.fifo_addr_top, TUSB_XFER_ISOCHRONOUS);

#if defined (BE_U1000) && (USBD_USE_DMA)
            uint8_t dma_ch = (epnum << 1);
            if (dma_ch <= (DMA_CH_MAX - 1U))
            {
                /* Use DMA channel for the selected endpoint */
                edpt->dma_ch = DMA_CH_IN_USE_FLAG | dma_ch;
            }
            else
            {
                /* Do not use DMA channel for the selected endpoint */
                edpt->dma_ch = 0x0U;
            }
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        }

#if defined(USBD_DPKTBUF)
        dcd.fifo_addr_top += edpt->max_packet_size << 1;
#else
        dcd.fifo_addr_top += edpt->max_packet_size;
#endif /* defined(USBD_DPKTBUF) */
    }
    else
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief Configures an isochronous endpoint according to descriptor.
 * @param rhport The USB peripheral number.
 * @param desc_ep The endpoint descriptor.
 * @return True if the endpoint has been configured successfully, otherwise false.
 */
bool dcd_edpt_iso_activate(uint8_t rhport, tusb_desc_endpoint_t const * desc_ep)
{
    (void)rhport;

    bool retval = true;

    uint8_t epnum = tu_edpt_number(desc_ep->bEndpointAddress);
    uint8_t dir = tu_edpt_dir(desc_ep->bEndpointAddress);

    if ((epnum > 0U) && (epnum < USBD_EP_MAX))
    {
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, dir);

        dcd_int_disable(rhport);
        edpt->max_packet_size = tu_edpt_packet_size(desc_ep);
        dcd_int_enable(rhport);
    }
    else
    {
        retval = false;
    }

    return retval; 
}

/**
 * @brief Configures OUT endpoint.
 * @param epnum The endpoint number.
 * @param max_packet_size The maximum amount of data that can be transferred
 *                        through the endpoint (in bytes).
 * @param fifo_addr The endpoint FIFO address.
 * @param xfer_type The endpoint transfer type.
 */
void epn_rx_configure(uint8_t epnum, uint16_t max_packet_size, uint16_t fifo_addr, uint8_t xfer_type)
{
    /* Store the current endpoint index */
    uint8_t old_index = USB->COMMON.INDEX & USB_INDEX_ENDPOINT;

    /* Select the endpoint registers */
    USB->COMMON.INDEX = epnum & USB_INDEX_ENDPOINT;

    /* Configure the endpoint maximum packet size */
    USB->INDEXED_EPCSR.PERIPH.RXMAXP = max_packet_size & USB_RXMAXP_MAXP;

    /* Set FIFO address */
    USB->CTRL.RXFIFOADD = (fifo_addr >> 3) & USB_RXFIFOADD_AD;

    /* Set FIFO size */
    USB->CTRL.RXFIFOSZ = (__CTZ(max_packet_size) - 3U) & USB_RXFIFOSZ_SZ;

#if defined(USBD_DPKTBUF)
    /* Enable double packet buffering */
    USB->CTRL.RXFIFOSZ |= USB_RXFIFOSZ_DPB;
#endif /* defined(USBD_DPKTBUF) */

    /* Flush FIFO */
    if (USB->INDEXED_EPCSR.PERIPH.RXCSRL & USB_RXCSRL_PERIPH_RXPKTRDY)
    {
        /* NOTE: It may be necessary to set this bit twice in succession
                 if double buffering is enabled. */
        USB->INDEXED_EPCSR.PERIPH.RXCSRL |= USB_RXCSRL_PERIPH_FLSHFIFO;
    }

    /* Reset the endpoint data toggle */
    USB->INDEXED_EPCSR.PERIPH.RXCSRL |= USB_RXCSRL_PERIPH_CLRDT;

    /* Configure transfer type */
    switch ((tusb_xfer_type_t)xfer_type)
    {
        case TUSB_XFER_ISOCHRONOUS:
        {
            /* Enable the RX endpoint for Isochronous transfer */
            USB->INDEXED_EPCSR.PERIPH.RXCSRH |= USB_RXCSRH_PERIPH_ISO;
        }
        break;

        case TUSB_XFER_BULK:
        {
            /* Enable the RX endpoint for Bulk transfer */
            USB->INDEXED_EPCSR.PERIPH.RXCSRH &= ~USB_RXCSRH_PERIPH_ISO;
        }
        break;

        case TUSB_XFER_INTERRUPT:
        {
            /* Enable the RX endpoint for Interrupt transfer */
            USB->INDEXED_EPCSR.PERIPH.RXCSRH &= ~USB_RXCSRH_PERIPH_ISO;
            /* Disable NYET handshakes for Interrupt endpoints */
            /* It has effect only for High-speed mode */
            USB->INDEXED_EPCSR.PERIPH.RXCSRH |= USB_RXCSRH_PERIPH_DISNYET;
        }
        break;

        case TUSB_XFER_CONTROL:
        default:
        {
            /* Should never reach here */
        }
        break;
    }

    /* Restore endpoint index */
    USB->COMMON.INDEX = old_index;

    /* The interrupt for OUT endpoint will be enabled later 
       after the receive buffer is allocated */
}

/**
 * @brief Configures IN endpoint.
 * @param epnum The endpoint number.
 * @param max_packet_size The maximum amount of data that can be transferred
 *                        through the endpoint (in bytes).
 * @param fifo_addr The endpoint FIFO address.
 * @param xfer_type The endpoint transfer type.
 */
void epn_tx_configure(uint8_t epnum, uint16_t max_packet_size, uint16_t fifo_addr, uint8_t xfer_type)
{
    /* Store the current endpoint index */
    uint8_t old_index = USB->COMMON.INDEX & USB_INDEX_ENDPOINT;

    /* Select the endpoint registers */
    USB->COMMON.INDEX = epnum & USB_INDEX_ENDPOINT;

    /* Configure the endpoint maximum packet size */
    USB->INDEXED_EPCSR.PERIPH.TXMAXP = max_packet_size & USB_TXMAXP_MAXP;

    /* Set FIFO address */
    USB->CTRL.TXFIFOADD = (fifo_addr >> 3) & USB_TXFIFOADD_AD;

    /* Set FIFO size */
    USB->CTRL.TXFIFOSZ = (__CTZ(max_packet_size) - 3U) & USB_TXFIFOSZ_SZ;

#if defined(USBD_DPKTBUF)
    /* Enable double packet buffering */
    USB->CTRL.TXFIFOSZ |= USB_TXFIFOSZ_DPB;
#endif /* defined(USBD_DPKTBUF) */

    /* Flush FIFO */
    if (USB->INDEXED_EPCSR.PERIPH.TXCSRL & USB_TXCSRL_PERIPH_TXPKTRDY)
    {
        /* NOTE: It may be necessary to set this bit twice in succession
                 if double buffering is enabled. */
        USB->INDEXED_EPCSR.PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_FLSHFIFO;
    }

    /* Reset the endpoint data toggle */
    USB->INDEXED_EPCSR.PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_CLRDT;

    /* Configure transfer type */
    switch ((tusb_xfer_type_t)xfer_type)
    {
        case TUSB_XFER_ISOCHRONOUS:
        {
            /* Enable the TX endpoint for Isochronous transfer */
            USB->INDEXED_EPCSR.PERIPH.TXCSRH |= USB_TXCSRH_PERIPH_ISO;
        }
        break;

        case TUSB_XFER_BULK:
        {
            /* Enable the TX endpoint for Bulk transfer */
            USB->INDEXED_EPCSR.PERIPH.TXCSRH &= ~USB_TXCSRH_PERIPH_ISO;
        }
        break;

        case TUSB_XFER_INTERRUPT:
        {
            /* Enable the TX endpoint for Interrupt transfer */
            USB->INDEXED_EPCSR.PERIPH.TXCSRH &= ~USB_TXCSRH_PERIPH_ISO;
        }
        break;

        case TUSB_XFER_CONTROL:
        default:
        {
            /* Should never reach here */
        }
        break;
    }

    /* Restore endpoint index */
    USB->COMMON.INDEX = old_index;

    /* Enable the endpoint interrupt */
    USB->COMMON.INTRTXE |= (1U << epnum);
}

/**
 * @brief Processes all the hardware generated events.
 * @param rhport The USB peripheral number.
 */
void dcd_int_handler(uint8_t rhport)
{
    /* Read and clear interrupt registers */
    uint8_t usbInt = USB->COMMON.INTRUSB & USB->COMMON.INTRUSBE;
    uint16_t rxInt = USB->COMMON.INTRRX & USB->COMMON.INTRRXE;
    uint16_t txInt = USB->COMMON.INTRTX & USB->COMMON.INTRTXE;

    if (usbInt & USB_INTRUSB_RESUMEIF)
    {
        /* Resume interrupt */
        dcd_event_bus_signal(rhport, DCD_EVENT_RESUME, true);
    }

    if (usbInt & USB_INTRUSB_SUSPIF)
    {
        /* Suspend interrupt */
        dcd_event_bus_signal(rhport, DCD_EVENT_SUSPEND, true);
    }

    if (usbInt & USB_INTRUSB_RESETIF)
    {
        /* Reset interrupt */
        /* The USB reset re-enables interrupts for all endpoints (doc: 3.2.5 and 3.2.6) */

        /* Close all non-control endpoints. It will disable all endpoint
           interrupts except the control endpoint and reset FIFO pointer. */
        /* dcd_edpt_close_all() will be called again on Set Configuration request. */
        dcd_edpt_close_all(rhport);

        /* Configure the control endpoint (EP0) */
        /* The control endpoint consumes two endpoint addresses (IN and OUT).
           Initialize two control structures for EP0. */
        epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);
        edpt->ep_addr = TUSB_DIR_IN_MASK;
        edpt->max_packet_size = CFG_TUD_ENDPOINT0_SIZE;
#if defined (BE_U1000) && (USBD_USE_DMA)
        edpt->dma_ch = DMA_CH_IN_USE_FLAG;
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */

        edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);
        edpt->ep_addr = 0x0U;
        edpt->max_packet_size = CFG_TUD_ENDPOINT0_SIZE;
#if defined (BE_U1000) && (USBD_USE_DMA)
        edpt->dma_ch = DMA_CH_IN_USE_FLAG;
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */

        /* Set EP0 state to IDLE */
        dcd.ep0_state = EP0_STATE_IDLE;

        /* Notify USB stack about reset from the host */
        dcd_event_bus_reset(rhport, (USB->COMMON.POWER & USB_POWER_HSMODE) ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL, true);
    }

    if (usbInt & USB_INTRUSB_SOFIF)
    {
        /* Start of Frame interrupt */
        dcd_event_bus_signal(rhport, DCD_EVENT_SOF, true);
    }

    /* Endpoint 0 interrupt */
    if (txInt & USB_INTRTX_EP0IF)
    {
        txInt &= ~USB_INTRTX_EP0IF;
        ep0_int_handler();
    }

    /* Tx Endpoint's interrupts */
    uint8_t i = 0U;
    while (txInt > 0U)
    {
        txInt >>= 1;
        i++;

        if (txInt & 0x1U)
        {
            epn_tx_int_handler(i);
        }
    }

    /* Rx Endpoint's interrupts */
    i = 0U;
    while (rxInt > 0U)
    {
        rxInt >>= 1;
        i++;

        if (rxInt & 0x1U)
        {
            epn_rx_int_handler(i);
        }
    }
}

/**
 * @brief Handles the received SETUP packet.
 */
void ep0_setup_packet_handler(void)
{
    /* A data packet has been received */

    /* If the EP0 is in IDLE state the only reason an interrupt
       can be generated is a SETUP packet received */

    /* Read data from FIFO, but do not acknowledge it until it is processed
       by the USB stack. It is safe to overwrite control request data here
       since the Data stage of the previous request must be finished to this
       moment. */
    *(uint32_t*)((uint32_t)&dcd.control_request) = USB->FIFO[0];
    *(uint32_t*)((uint32_t)&dcd.control_request + sizeof(uint32_t)) = USB->FIFO[0];

    /* Notify USB stack about SETUP packet received. The corresponding event
       will be enqueued and dcd_edpt_xfer() will be invoked in response to this
       notification to initialize the Data stage of the transfer. */
    dcd_event_setup_received(BOARD_TUD_RHPORT, &dcd.control_request.bmRequestType, true);

    /* The EP0 state will not be switched here because there may be
       events in the queue that have not been processed yet. Instead,
       setup event will be enqueued and processed when its turn comes. */
}

/**
 * @brief Endpoint 0 interrupt handler.
 */
void ep0_int_handler(void)
{
    /* Select EP0 registers */
    USB->COMMON.INDEX = 0U;

    uint8_t status = USB->EPCSR[0].PERIPH.CSR0L;

    if (status & USB_CSR0L_PERIPH_SENTSTALL)
    {
        /* Control transfer has ended due to a protocol violation */
        /* Abort processing the current control transfer */

        /* Clear SentStall bit */
        USB->EPCSR[0].PERIPH.CSR0L &= ~USB_CSR0L_PERIPH_SENTSTALL;

        /* Set EP0 state to IDLE */
        dcd.ep0_state = EP0_STATE_IDLE;
    }

    if (status & USB_CSR0L_PERIPH_SETUPEND)
    {
        /* The control transfer has ended prematurely before DataEnd bit has been set */
        /* Abort processing the current control transfer */

        /* Clear SetupEnd bit */
        USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVSSETEND;

        /* Set EP0 state to IDLE */
        dcd.ep0_state = EP0_STATE_IDLE;
    }

    switch (dcd.ep0_state)
    {
        case EP0_STATE_IDLE:
        {
            /* Idle state, wait for SETUP packet */
            if (status & USB_CSR0L_PERIPH_RXPKTRDY)
            {
                ep0_setup_packet_handler();
            }
        }
        break;

        case EP0_STATE_DATA_IN:
        {
            /* Data IN state */
            epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

            /* The IN data packet has been sent to host */
            edpt->xfer.xferred += edpt->xfer.last_packet_size;
            edpt->xfer.transferred += edpt->xfer.last_packet_size;

            /* The xfer might be complete at this point, but not yet entire USB transfer */
            if (edpt->xfer.xferred >= edpt->xfer.xfer_len)
            {
                /* Notify the USB stack that the current xfer is complete, but
                   there is more data in this USB transfer. Another xfer will be
                   allocated for the rest of data. */
                dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);
            }
            else
            {
                /* Send the next packet */
                ep0_data_in(edpt);
            }
        }
        break;

        case EP0_STATE_DATA_OUT:
        {
            /* Data OUT state */
            if (status & USB_CSR0L_PERIPH_RXPKTRDY)
            {
                /* Data OUT token has been received */
                epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);

                edpt->xfer.last_packet_size = 0U;

#if defined (BE_U1000) && (USBD_USE_DMA)
                if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
                {
                    if (edpt->xfer.xfer_buf != NULL)
                    {
                        edpt->xfer.last_packet_size = tu_min16(USB->EPCSR[0].PERIPH.RXCOUNT, edpt->xfer.xfer_len - edpt->xfer.xferred);

                        uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

                        USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                        USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
                        USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                                       USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                       USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
                    }
                }
                else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */ 
                {
                    if (edpt->xfer.xfer_buf != NULL)
                    {
                        edpt->xfer.last_packet_size = rx_fifo_read(0U,
                                                                   (uint8_t*)((uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred),
                                                                   edpt->xfer.xfer_len - edpt->xfer.xferred);
                        edpt->xfer.xferred += edpt->xfer.last_packet_size;
                        edpt->xfer.transferred += edpt->xfer.last_packet_size;
                    }

                    if ((edpt->xfer.xferred >= edpt->xfer.xfer_len) ||
                        (edpt->xfer.last_packet_size < edpt->max_packet_size))
                    {
                        /* Notify the USB stack about the end of the xfer */
                        dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);
                    }
                    else
                    {
                        /* Expect more data packets */
                        USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;
                    }
                }
            }
        }
        break;

        case EP0_STATE_STATUS_IN:
        {
            /* Status IN state */

            /* This interrupt is generated to indicate that the request has completed.
               No more interrupts will be generated for this transfer. */

            epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

            /* Notify USB stack about the end of the STATUS stage */
            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, 0U, XFER_RESULT_SUCCESS, true);

            /* Switch to IDLE state */
            dcd.ep0_state = EP0_STATE_IDLE;
        }
        break;

        case EP0_STATE_STATUS_OUT:
        {
            /* Status OUT state */

            /* This interrupt is generated to indicate that the request has completed.
               No more interrupts will be generated for this transfer. */

            if (status & USB_CSR0L_PERIPH_RXPKTRDY)
            {
                /* The new SETUP packet may come in while EP0 is in STATUS OUT
                   state. It will be received and enqueued, but it will be
                   processed only after the events belonging to the STATUS stage
                   of the current transfer are completed. */
                ep0_setup_packet_handler();
            }
            else
            {
                epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

                /* This check is the guard to ensure that the following routine
                   is executed only once after the last packet has been sent */
                if (edpt->xfer.xferred < edpt->xfer.xfer_len)
                {   
                    /* The last IN data packet has been sent */
                    edpt->xfer.xferred += edpt->xfer.last_packet_size;
                    edpt->xfer.transferred += edpt->xfer.last_packet_size;

                    /* Notify USB stack about the end of the DATA stage. The corresponding
                       event will be enqueued and dcd_edpt_xfer() will be invoked in response
                       to this notification to initialize the Status stage of the transfer. */
                    dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);

                    /* Notify USB stack about the end of the STATUS stage. The corresponding
                       event will be enqueued, but dcd_edpt_xfer() will not be invoked in response
                       to this notification since there is no more transactions in this transfer
                       to schedule. Even though this step does not require user interaction,
                       it is necessary for the USB stack to finalize the transfer and release
                       allocated resources. */
                    edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);
                    dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, 0U, XFER_RESULT_SUCCESS, true);
                }
            }
        }
        break;

        case EP0_STATE_STATUS_SET_ADDRESS:
        {
            /* Status IN state, handle Set Address request */

            /* Control transfer is complete, the device address can be changed */
            USB->COMMON.FADDR = dcd.dev_addr & USB_FADDR_FUNC;

            /* Switch to IDLE state */
            dcd.ep0_state = EP0_STATE_IDLE;

            /* For Set Address request the STATUS stage is responsibility of DCD.
               It was not planned by the USB stack, therefore there is no need
               no notify stack about completion. */
        }
        break;

        default:
        {
            /* Switch to IDLE state */
            dcd.ep0_state = EP0_STATE_IDLE;
        }
        break;
    }
}

/**
 * @brief RX Endpoint 1 to 15 interrupt handler.
 * @param epnum The endpoint number.
 */
void epn_rx_int_handler(uint8_t epnum)
{
    uint8_t status = USB->EPCSR[epnum].PERIPH.RXCSRL;

    if (status & USB_RXCSRL_PERIPH_SENTSTALL)
    {
        /* Clear SentStall bit */
        USB->EPCSR[epnum].PERIPH.RXCSRL &= ~USB_RXCSRL_PERIPH_SENTSTALL;
    }

    if (status & USB_RXCSRL_PERIPH_RXPKTRDY)
    {
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_OUT);

#ifdef USBD_SPEED_TEST_RX
        /* USB RX speed test */
#if defined (BE_U1000) && (USBD_USE_DMA)
        if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
        {
            edpt->xfer.last_packet_size = tu_min16(USB->EPCSR[epnum].PERIPH.RXCOUNT, edpt->xfer.xfer_len);

            uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

            USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + (uint32_t)RAM_ACCESS_OFFSET;
            USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
            USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                           USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                           USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
        }
        else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        {
            /* Read data from FIFO */
            edpt->xfer.last_packet_size = rx_fifo_read(epnum, edpt->xfer.xfer_buf, edpt->xfer.xfer_len);

            /* Clear RxPktRdy flag */
            USB->EPCSR[epnum].PERIPH.RXCSRL &= ~USB_RXCSRL_PERIPH_RXPKTRDY;
        }
#else

#if defined (BE_U1000) && (USBD_USE_DMA)
        if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
        {
            /* Configure DMA transfer */
            edpt->xfer.last_packet_size = tu_min16(USB->EPCSR[epnum].PERIPH.RXCOUNT, edpt->xfer.xfer_len - edpt->xfer.xferred);

            uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

            USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
            USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
            USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                           USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                           USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
        }
        else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        {
            /* Copy data from FIFO */
            edpt->xfer.last_packet_size = rx_fifo_read(epnum,
                                                       (uint8_t*)((uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred),
                                                       edpt->xfer.xfer_len - edpt->xfer.xferred);

            edpt->xfer.xferred += edpt->xfer.last_packet_size;

            /* RxPktRdy flag will be cleared later in dcd_edpt_xfer() after the next
               data buffer is allocated. Until then the function will respond with NAK. */

            /* Notify USB stack immediately, even if the buffer size
               allocated is less then the size of the data received */
            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);
        }
#endif /* USBD_SPEED_TEST_RX */
    }
}

/**
 * @brief TX Endpoint 1 to 15 interrupt handler.
 * @param epnum The endpoint number.
 */
void epn_tx_int_handler(uint8_t epnum)
{
    uint8_t status = USB->EPCSR[epnum].PERIPH.TXCSRL;

    if (status & USB_TXCSRL_PERIPH_SENTSTALL)
    {
        /* Clear SentStall bit */
        USB->EPCSR[epnum].PERIPH.TXCSRL &= ~USB_TXCSRL_PERIPH_SENTSTALL;
    }
    else
    {
#if defined (USBD_SPEED_TEST_RX)
        /* USB RX speed test */
#elif defined (USBD_SPEED_TEST_TX)
        /* USB TX speed test */
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_IN);

#if defined (BE_U1000) && (USBD_USE_DMA)
        if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
        {
            /* Configure DMA transfer */
            uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

            USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + (uint32_t)RAM_ACCESS_OFFSET;
            USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
            USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                           USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                           USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;

#if defined(USBD_DPKTBUF)
            /* Disable the TX endpoint interrupt until DMA transfer is complete */
            USB->COMMON.INTRTXE &= ~(0x1U << epnum);
#endif /* USBD_DPKTBUF */
        }
        else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
        {
            tx_fifo_write(epnum, edpt->xfer.xfer_buf, edpt->xfer.last_packet_size);
            USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
        }

#else
        /* The data packet has been sent */
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_IN);

        edpt->xfer.xferred += edpt->xfer.last_packet_size;

        if (edpt->xfer.xferred >= edpt->xfer.xfer_len)
        {
            /* No more data to send, notify the USB stack about completion */
            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);
        }
        else
        {
            /* Send the next data packet */
            edpt->xfer.last_packet_size = tu_min16(edpt->max_packet_size, edpt->xfer.xfer_len - edpt->xfer.xferred);

#if defined (BE_U1000) && (USBD_USE_DMA)
            if (edpt->dma_ch & DMA_CH_IN_USE_FLAG)
            {
                uint8_t dma_ch = edpt->dma_ch & DMA_CH_NUM_MASK;

                USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                USB->DMA.CH[dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
                USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                               USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                               USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
            }
            else
#endif /* defined (BE_U1000) && (USBD_USE_DMA) */
            {
                tx_fifo_write(epnum,
                              (uint8_t*)((uint32_t)edpt->xfer.xfer_buf + edpt->xfer.xferred),
                              edpt->xfer.last_packet_size);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
            }
        }
#endif /* USBD_SPEED_TEST_RX */
    }
}

/**
 * @brief Writes the endpoint FIFO.
 * @param epnum The endpoint number.
 * @param buffer The pointer to data buffer.
 * @param count The number of bytes to write.
 */
void tx_fifo_write(uint8_t epnum, uint8_t const * buffer, uint16_t count)
{
    if (buffer != NULL)
    {
        /* The endpoint buffer alignment is defined by CFG_TUSB_MEM_ALIGN macro 
        in tusb_option.h. Make sure the alignment size is 4 bytes for optimal
        performance. */
        uint32_t *buffer_word = (uint32_t*)buffer;

        volatile uint8_t * fifo_reg = (volatile uint8_t *)(&USB->FIFO[epnum]);
        volatile uint32_t * fifo_reg_word = (volatile uint32_t *)(&USB->FIFO[epnum]);

        uint16_t i = 0U;

        /* Write FIFO by word whenever possible */
        uint16_t count_word = count / sizeof(uint32_t);
        while (i < count_word)
        {
            *fifo_reg_word = buffer_word[i];
            i++;
        }

        i *= sizeof(uint32_t);

        /* Write the rest of data by byte */
        while (i < count)
        {
            *fifo_reg = buffer[i];
            i++;
        }
    }
}

/**
 * @brief Reads the endpoint FIFO.
 * @param epnum The endpoint number.
 * @param buffer The pointer to data buffer.
 * @param max_bytes The maximal number of bytes to read.
 * @return The number of bytes have been read.
 */
uint16_t rx_fifo_read(uint8_t epnum, uint8_t * buffer, uint16_t max_bytes)
{
    uint16_t count = USB->EPCSR[epnum].PERIPH.RXCOUNT;
    uint16_t to_read = (count > max_bytes) ? max_bytes : count;

    if (buffer != NULL)
    {
        /* The endpoint buffer alignment is defined by CFG_TUSB_MEM_ALIGN macro 
        in tusb_option.h. Make sure the alignment size is 4 bytes for optimal
        performance. */
        uint32_t *buffer_word = (uint32_t*)buffer;

        volatile uint8_t * fifo_reg = (volatile uint8_t *)(&USB->FIFO[epnum]);
        volatile uint32_t * fifo_reg_word = (volatile uint32_t *)(&USB->FIFO[epnum]);

        uint16_t i = 0U;

        /* Read FIFO by word whenever possible */
        uint16_t to_read_word = to_read / sizeof(uint32_t);

        while (i < to_read_word)
        {
            buffer_word[i] = *fifo_reg_word;
            i++;
        }

        i *= sizeof(uint32_t);

        /* Read the rest of data by byte */
        while (i < to_read)
        {
            buffer[i] = *fifo_reg;
            i++;
        }
    }
    else
    {
        to_read = 0U;
    }

    return to_read;
}

void __attribute__ ((interrupt)) USB_IRQHandler(void)
{
    dcd_int_handler(BOARD_TUD_RHPORT);
}

#if defined (BE_U1000) && (USBD_USE_DMA)

void __attribute__ ((interrupt)) USB_DMA_IRQHandler(void)
{
    volatile uint8_t dmaInt = USB->DMA.DMA_INTR;

    uint8_t i = 0U;
    while (dmaInt > 0U)
    {
        if (dmaInt & 0x1U)
        {
            if (i == 0U)
            {
                /* Control endpoint */
                switch (dcd.ep0_state)
                {
                    case EP0_STATE_DATA_IN:
                    {
                        /* Data IN state */
                        epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

                        if (((edpt->xfer.transferred + edpt->xfer.last_packet_size) >= dcd.control_request.wLength) ||
                            (edpt->xfer.last_packet_size < edpt->max_packet_size))
                        {
                            /* Switch to STATUS OUT state */
                            dcd.ep0_state = EP0_STATE_STATUS_OUT;

                            /* Send the last data packet */
                            USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_TXPKTRDY | USB_CSR0L_PERIPH_DATAEND);

                            /* When the last packet has been sent to the host, no more Endpoint 0
                               interrupts will be generated in the Data stage. The host moves to
                               the Status stage of the request, and another Endpoint 0 interrupt
                               will be generated to indicate that the request has completed. */
                        }
                        else
                        {
                            /* Send the data packet, more data packets expected */
                            USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_TXPKTRDY;

                            /* When the packet has been sent to the host, another Endpoint 0 interrupt
                               will be generated and the next data packet can be written to the FIFO */
                        }
                    }
                    break;

                    case EP0_STATE_DATA_OUT:
                    {
                        /* Data OUT state */
                        epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);

                        edpt->xfer.xferred += edpt->xfer.last_packet_size;
                        edpt->xfer.transferred += edpt->xfer.last_packet_size;

                        if ((edpt->xfer.xferred >= edpt->xfer.xfer_len) ||
                            (edpt->xfer.last_packet_size < edpt->max_packet_size))
                        {
                            /* All the expected data packets have been received */

                            /* The USB stack will be notified about the end of the DATA stage.
                               The stack will prepare STATUS stage and call dcd_edpt_xfer().
                               The last data packet will be acknowledged there and the USB
                               core will proceed with the transfer complete interrupt. */

                            /* Notify stack about the end of the DATA stage */
                            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);
                        }
                        else
                        {
                            /* Expect more data packets */
                            USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;
                        }
                    }
                    break;

                    default:
                    {
                        /* No action */
                    }
                    break;
                }
            }
            else if ((i & 0x1U) != 0x0U)
            {
                /* RX endpoint */
                uint8_t epnum = ((i + 1U) >> 1);

#ifdef USBD_SPEED_TEST_RX
                /* Clear RxPktRdy flag */
                USB->EPCSR[epnum].PERIPH.RXCSRL &= ~USB_RXCSRL_PERIPH_RXPKTRDY;
#else
                epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_OUT);

                edpt->xfer.xferred += edpt->xfer.last_packet_size;
                
                /* Notify USB stack immediately, even if the buffer size
                   allocated is less then the size of the data received */
                dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.xferred, XFER_RESULT_SUCCESS, true);
#endif /* USBD_SPEED_TEST_RX */
            }
            else
            {
                /* TX endpoint */
#ifdef USBD_SPEED_TEST_RX
                /* USB RX speed test */
#elif defined (USBD_SPEED_TEST_TX)
                /* USB TX speed test */
                uint8_t epnum = (i >> 1);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;

#if defined(USBD_DPKTBUF)
                /* Re-enable the TX endpoint interrupt */
                USB->COMMON.INTRTXE |= (0x1U << epnum);
#endif /* USBD_DPKTBUF */
   
#else
                uint8_t epnum = (i >> 1);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
#endif /* USBD_SPEED_TEST_RX */
            }
        }

        dmaInt >>= 1;
        i++;
    } 
}

#endif /* defined (BE_U1000) && (USBD_USE_DMA) */

#endif
