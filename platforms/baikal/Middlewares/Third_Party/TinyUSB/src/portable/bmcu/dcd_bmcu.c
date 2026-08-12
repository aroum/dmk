/**
 * *****************************************************************************
 *  @file       dcd_bmcu.c
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB driver
 *  @version    2.2.0
 *  @date       2026.04.02
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#include "tusb_option.h"

#if CFG_TUD_ENABLED && CFG_TUSB_MCU == OPT_MCU_BMCU

#if defined (BE_U1000)
#define USB_DMA
#endif /* BE_U1000 */

#if defined(USB_BW_TEST)

#define USB_BW_TEST_RX

#ifndef USB_BW_TEST_RX
#define USB_BW_TEST_TX
#endif /* USB_BW_TEST_RX */

#define USB_DPKTBUF

#endif /* USB_BW_TEST */

#include "device/dcd.h"
#include "bmcu_common.h"

#ifndef EP_MAX
#define EP_MAX      6U  /*!< The maximum number of endpoints (control and non-control) */
#endif

/** @brief Endpoint 0 stage definitions */
typedef enum {
    EP0_STAGE_IDLE = 0,             /*!< Idle stage, wait for SETUP transaction */
    EP0_STAGE_SETUP_ZERO_DATA,      /*!< Setup stage, control transfer without Data stage */
    EP0_STAGE_DATA_IN,              /*!< Data IN stage */
    EP0_STAGE_DATA_OUT,             /*!< Data OUT stage */
    EP0_STAGE_STATUS_IN,            /*!< Status IN stage */
    EP0_STAGE_STATUS_OUT,           /*!< Status OUT stage */
    EP0_STAGE_STATUS_SET_ADDRESS    /*!< Status stage, handle Set Address request (special case of Status IN) */
} ep0_stage_t;

/** @brief Transfer control structure */
typedef struct {
    uint8_t* buffer;            /*!< Pointer to data buffer provided by USB stack (only for a single packet sent or received, NOT entire transfer) */
    uint16_t datalen;           /*!< Transaction data length in bytes (a single transaction, NOT entire transfer) */
    uint16_t last_packet_size;  /*!< The size of the last packet sent or received in bytes */
    uint16_t transferred;       /*!< The number of bytes transferred in data stage of the current transfer */
} xfer_ctrl_t;

/** @brief Endpoint control structure */
typedef struct {
    uint8_t     ep_addr;            /*!< The address of the endpoint (bEndpointAddress) */
    uint16_t    max_packet_size;    /*!< The maximum packet size endpoint is capable of sending or receiving (wMaxPacketSize) */
    uint16_t    fifo_size;          /*!< The size of FIFO associated with the endpoint */
    xfer_ctrl_t xfer;               /*!< The transfer control structure */
#if defined (BE_U1000) && defined (USB_DMA)
    uint8_t     dma_ch;             /*!< DMA channel assigned to the endpoint */
#endif /* defined (BE_U1000) && defined (USB_DMA) */
} epn_ctrl_t;

/** @brief USB Device Class Driver control structure */
typedef struct
{
    uint8_t dev_addr;                           /*!< Device address */
    uint16_t fifo_addr_top;                     /*!< Current FIFO address used for endpoint FIFO allocation (in units of 8 bytes) */
    tusb_control_request_t control_request;     /*!< Control request */
    ep0_stage_t ep0_stage;                      /*!< Endpoint 0 stage */
    epn_ctrl_t ep[EP_MAX][2];                   /*!< The endpoint control structure */
} dcd_ctrl_t;

static dcd_ctrl_t dcd;

/** @brief The macro to get the EP transfer control structure pointer */
#define EP_CTRL_BASE(epnum, dir)        &dcd.ep[epnum][dir]

/** @brief Configures OUT endpoint. */
static void epn_rx_configure(uint8_t endpoint,
                             uint16_t endpointSize,
                             uint16_t fifoAddress,
                             uint8_t fifoSize,
                             uint8_t transferType);
/** @brief Configures IN endpoint. */
static void epn_tx_configure(uint8_t endpoint,
                             uint16_t endpointSize,
                             uint16_t fifoAddress,
                             uint8_t fifoSize,
                             uint8_t transferType);

/** @brief Endpoint 0 interrupt handler */
static void ep0_int_handler(void);
/** @brief Endpoint 1 to 15 RX interrupt handler */
static void epn_rx_int_handler(uint8_t epnum);
/** @brief Endpoint 1 to 15 TX interrupt handler */
static void epn_tx_int_handler(uint8_t epnum);

#if !defined (BE_U1000) || !defined (USB_DMA)
/** @brief Writes the endpoint FIFO. */
static void tx_fifo_write(uint8_t epnum, uint8_t const * buffer, size_t count);
/** @brief Reads the endpoint FIFO. */
static uint16_t rx_fifo_read(uint8_t epnum, uint8_t * buffer);
#endif /* !defined (BE_U1000) || !defined (USB_DMA) */

/**
 * @brief Initializes the USB peripheral for device mode and enables it.
 * @note This function should enable internal D+/D- pull-up for enumeration.
 * @param rhport The USB peripheral number.
 */
bool dcd_init(uint8_t rhport, const tusb_rhport_init_t* rh_init)
{
    (void)rh_init;

    /* Reset */
    USB->CTRL.SOFT_RST |= (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST);
    while ((USB->CTRL.SOFT_RST & (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST)) != 0U);
    
    /* Reserve EP0 FIFO */
    dcd.fifo_addr_top = (64U >> 3);

    /* Disable endpoint interrupts */
    USB->COMMON.INTRRXE = 0x0000U;
    USB->COMMON.INTRTXE = 0x0000U;

    /* Enable Reset/Suspend/Resume interrupts */
    USB->COMMON.INTRUSBE = USB_INTRUSBE_RESETIE |
                           USB_INTRUSBE_RESUMEIE |
                           USB_INTRUSBE_SUSPIE;

    /* Configure USB interrupts in CLIC */
#ifndef CORE_BR310S14
    CLIC_ConfigIRQ(CLIC_USB_IRQn,                   /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   1U,                              /* Level */
                   1U,                              /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */

#if defined (BE_U1000) && defined (USB_DMA)
    CLIC_ConfigIRQ(CLIC_USB_DMA_IRQn,               /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   1U,                              /* Level */
                   1U,                              /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */
#endif /* defined (BE_U1000) && defined (USB_DMA) */

#endif

    dcd_connect(rhport);

    return true;
}

/**
 * @brief Enables the USB device interrupts.
 * @param rhport The USB peripheral number.
 */
void dcd_int_enable(uint8_t rhport)
{
    (void)rhport;

#ifndef CORE_BR310S14
    CLIC_EnableIRQ(CLIC_USB_IRQn);
#if defined (BE_U1000) && defined (USB_DMA)
    CLIC_EnableIRQ(CLIC_USB_DMA_IRQn);
#endif /* defined (BE_U1000) && defined (USB_DMA) */
#endif
}

/**
 * @brief Disables the USB device interrupts.
 * @param rhport The USB peripheral number.
 */
void dcd_int_disable(uint8_t rhport)
{
    (void)rhport;

#ifndef CORE_BR310S14
    CLIC_DisableIRQ(CLIC_USB_IRQn);
#if defined (BE_U1000) && defined (USB_DMA)
    CLIC_DisableIRQ(CLIC_USB_DMA_IRQn);
#endif /* defined (BE_U1000) && defined (USB_DMA) */
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
       when SET_ADDRESS request received */

    /* Store the address in USB device control structure */
    /* It will be written to register on control request completion */
    dcd.dev_addr = dev_addr;

    dcd.ep0_stage = EP0_STAGE_STATUS_SET_ADDRESS;
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

    if ((epnum > 0U) && (epnum < EP_MAX))
    {
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, dir);

        edpt->ep_addr = desc_edpt->bEndpointAddress;

        edpt->max_packet_size = tu_edpt_packet_size(desc_edpt);
        edpt->fifo_size = edpt->max_packet_size;    /* FIFO size is set to buffer one packet */
        
        if (dir == TUSB_DIR_OUT)
        {
#if defined (BE_U1000) && defined (USB_DMA)
            edpt->dma_ch = epnum;
#endif /* defined (BE_U1000) && defined (USB_DMA) */

            epn_rx_configure(epnum,
                                edpt->max_packet_size,
                                dcd.fifo_addr_top,
                                __CTZ(edpt->fifo_size) - 3U,
                                desc_edpt->bmAttributes.xfer);
#if defined(USB_BW_TEST_RX) && defined(USB_DPKTBUF)
            dcd.fifo_addr_top += ((edpt->fifo_size + 7U) >> 3) << 1;
#else
            dcd.fifo_addr_top += (edpt->fifo_size + 7U) >> 3;
#endif /* defined(USB_BW_TEST_RX) && defined(USB_DPKTBUF) */
        }
        else
        {
#if defined (BE_U1000) && defined (USB_DMA)
            edpt->dma_ch = epnum + 1U;
#endif /* defined (BE_U1000) && defined (USB_DMA) */

            epn_tx_configure(epnum,
                                edpt->max_packet_size,
                                dcd.fifo_addr_top,
                                __CTZ(edpt->fifo_size) - 3U,
                                desc_edpt->bmAttributes.xfer);
#if defined(USB_BW_TEST_TX) && defined(USB_DPKTBUF)
            dcd.fifo_addr_top += ((edpt->fifo_size + 7U) >> 3) << 1;
#else
            dcd.fifo_addr_top += (edpt->fifo_size + 7U) >> 3;
#endif /* defined(USB_BW_TEST_TX) && defined(USB_DPKTBUF) */
        }
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

    if ((epnum > 0U) && (epnum < EP_MAX))
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
    USB->COMMON.INTRRXE &= 0x0001U;
    USB->COMMON.INTRTXE &= 0x0001U;

    /* Reset EP FIFO pointer and reserve EP0 FIFO */
    dcd.fifo_addr_top = (64U >> 3);
}

/**
 * @brief Configures the peripheral to send or receive data from the host.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 * @param buffer The data buffer.
 * @param total_bytes The buffer length.
 * @return True if configured successfully, otherwise false.
 */
bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t * buffer, uint16_t total_bytes)
{
    (void)rhport;

    uint8_t epnum = tu_edpt_number(ep_addr);
    uint8_t dir = tu_edpt_dir(ep_addr);

    epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, dir);

    /* It's responsibility of USB stack to call the function with proper
       parameter values. So, only put asserts here for debug. */
    TU_ASSERT(buffer != NULL);
    TU_ASSERT(total_bytes <= edpt->max_packet_size);

    edpt->xfer.buffer = buffer;
    edpt->xfer.datalen = total_bytes;

    if (epnum == 0U)
    {
        /* Endpoint 0 (Control transfer) */
        switch (dcd.ep0_stage)
        {
            case EP0_STAGE_SETUP_ZERO_DATA:
            {
                /* Acknowledge SETUP transaction */
                dcd.ep0_stage = EP0_STAGE_STATUS_IN;
                USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_SVCRPR | USB_CSR0L_PERIPH_DATAEND);
            }
            break;

            case EP0_STAGE_DATA_IN:
            {
                /* Data IN stage */

                /* Submit a single IN transaction */
                edpt->xfer.last_packet_size = tu_min16(edpt->max_packet_size, edpt->xfer.datalen);

#if defined (BE_U1000) && defined (USB_DMA)
                USB->DMA.CH[edpt->dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.buffer;
                USB->DMA.CH[edpt->dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
                USB->DMA.CH[edpt->dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                                     USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                     USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
                edpt->xfer.transferred += edpt->xfer.last_packet_size;
#else
                tx_fifo_write(epnum, edpt->xfer.buffer, edpt->xfer.last_packet_size);
                edpt->xfer.transferred += edpt->xfer.last_packet_size;

                /* On an input request, a device must never return more data
                    than is indicated by the wLength value; it may return less */
                if ((edpt->xfer.transferred == dcd.control_request.wLength) ||
                    (edpt->xfer.last_packet_size < edpt->max_packet_size))
                {
                    /* The last data packet */
                    USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_TXPKTRDY | USB_CSR0L_PERIPH_DATAEND);
                }
                else
                {
                    /* Not the last data packet */
                    USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_TXPKTRDY;
                }
#endif /* defined (BE_U1000) && defined (USB_DMA) */
            }
            break;

            case EP0_STAGE_DATA_OUT:
            {
                /* Acknowledge the previous transaction */
                USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;
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
            if ((edpt->xfer.datalen > 0U) || 
                (edpt->xfer.last_packet_size == edpt->max_packet_size))
            {
                /* Sent packet if there is data to send 
                   or ZLP if the last packet was the max size */
                edpt->xfer.last_packet_size = tu_min16(edpt->max_packet_size, edpt->xfer.datalen);

#if defined (BE_U1000) && defined (USB_DMA)
                USB->DMA.CH[edpt->dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.buffer;
                USB->DMA.CH[edpt->dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
                USB->DMA.CH[edpt->dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                                     USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                     USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
#else
                tx_fifo_write(epnum, edpt->xfer.buffer, edpt->xfer.last_packet_size);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
#endif /* defined (BE_U1000) && defined (USB_DMA) */
            }
        }
    }

    return true;
}

/**
 * @brief Submits a transfer using FIFO.
 * @param rhport The USB peripheral number.
 * @param ep_addr The endpoint address.
 * @param ff The FIFO.
 * @param total_bytes The data length.
 * @return True on success, otherwise false.
 */
bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes)
{
    (void)rhport;
    (void)ep_addr;
    (void)ff;
    (void)total_bytes;

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
 * @brief Configures OUT endpoint.
 * @param endpoint The endpoint number.
 * @param endpointSize The endpoint maximum packet size (in bytes).
 * @param fifoAddress The endpoint FIFO address (in units of 8 bytes).
 * @param fifoSize The endpoint FIFO size (in units of 8 bytes).
 * @param transferType The endpoint transfer type.
 */
void epn_rx_configure(uint8_t endpoint,
                      uint16_t endpointSize,
                      uint16_t fifoAddress,
                      uint8_t fifoSize,
                      uint8_t transferType)
{
    /* Store the current endpoint index */
    uint8_t old_index = USB->COMMON.INDEX & USB_INDEX_ENDPOINT;

    /* Select the endpoint registers */
    USB->COMMON.INDEX = endpoint & USB_INDEX_ENDPOINT;

    /* Configure the endpoint maximum packet size */
    USB->INDEXED_EPCSR.PERIPH.RXMAXP = endpointSize & USB_RXMAXP_MAXP;

    /* Set FIFO address */
    USB->CTRL.RXFIFOADD = fifoAddress & USB_RXFIFOADD_AD;

    /* Set FIFO size */
    USB->CTRL.RXFIFOSZ = fifoSize & USB_RXFIFOSZ_SZ;

#if defined(USB_BW_TEST_RX) && defined(USB_DPKTBUF)
    /* Enable Double Packet Buffer for RX Endpoint */
    USB->CTRL.RXFIFOSZ |= USB_RXFIFOSZ_DPB;
#endif /* defined(USB_BW_TEST_RX) && defined(USB_DPKTBUF) */

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
    switch ((tusb_xfer_type_t)transferType)
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
 * @param endpoint The endpoint number.
 * @param endpointSize The endpoint maximum packet size (in bytes).
 * @param fifoAddress The endpoint FIFO address (in units of 8 bytes).
 * @param fifoSize The endpoint FIFO size (in units of 8 bytes).
 * @param transferType The endpoint transfer type.
 */
void epn_tx_configure(uint8_t endpoint,
                      uint16_t endpointSize,
                      uint16_t fifoAddress,
                      uint8_t fifoSize,
                      uint8_t transferType)
{
    /* Store the current endpoint index */
    uint8_t old_index = USB->COMMON.INDEX & USB_INDEX_ENDPOINT;

    /* Select the endpoint registers */
    USB->COMMON.INDEX = endpoint & USB_INDEX_ENDPOINT;

    /* Configure the endpoint maximum packet size */
    USB->INDEXED_EPCSR.PERIPH.TXMAXP = endpointSize & USB_TXMAXP_MAXP;

    /* Set FIFO address */
    USB->CTRL.TXFIFOADD = fifoAddress & USB_TXFIFOADD_AD;

    /* Set FIFO size */
    USB->CTRL.TXFIFOSZ = fifoSize & USB_TXFIFOSZ_SZ;

#if defined(USB_BW_TEST_TX) && defined(USB_DPKTBUF)
    /* Enable Double Packet Buffer for TX Endpoint */
    USB->CTRL.TXFIFOSZ |= USB_TXFIFOSZ_DPB;
#endif /* defined(USB_BW_TEST_TX) && defined(USB_DPKTBUF) */

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
    switch ((tusb_xfer_type_t)transferType)
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
    USB->COMMON.INTRTXE |= (1U << endpoint);
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
           Initialize two control structures for EP. */
        epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);
        edpt->ep_addr = TUSB_DIR_IN_MASK | 0x0U;
        edpt->max_packet_size = CFG_TUD_ENDPOINT0_SIZE;
        edpt->fifo_size = edpt->max_packet_size;

        edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);
        edpt->ep_addr = 0x0U;
        edpt->max_packet_size = CFG_TUD_ENDPOINT0_SIZE;
        edpt->fifo_size = edpt->max_packet_size;

        /* Set EP0 state to IDLE */
        dcd.ep0_stage = EP0_STAGE_IDLE;

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
        dcd.ep0_stage = EP0_STAGE_IDLE;
    }

    if (status & USB_CSR0L_PERIPH_SETUPEND)
    {
        /* The control transfer has ended prematurely before DataEnd bit has been set */
        /* Abort processing the current control transfer */

        /* Clear SetupEnd bit */
        USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVSSETEND;

        /* Set EP0 state to IDLE */
        dcd.ep0_stage = EP0_STAGE_IDLE;
    }

    switch (dcd.ep0_stage)
    {
        case EP0_STAGE_IDLE:
        {
            /* Idle stage, wait for Status packet */
            if (status & USB_CSR0L_PERIPH_RXPKTRDY)
            {
                /* Data packet has been received */
                for (uint8_t i = 0U; i < 2U; i++)
                {
                    *(uint32_t*)((uint32_t)&dcd.control_request + sizeof(uint32_t) * i) = USB->FIFO[0];
                }

                if (dcd.control_request.bmRequestType_bit.direction == TUSB_DIR_OUT)
                {
                    /* ServicedRxPktRdy and DataEnd is not set yet. It will be set
                       later when OUT xfer is configured. Till then NAKs will hold
                       incoming data. */
                    dcd.ep0_stage = (dcd.control_request.wLength == 0U) ?
                                    EP0_STAGE_SETUP_ZERO_DATA :
                                    EP0_STAGE_DATA_OUT;

                    epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);
                    edpt->xfer.transferred = 0U;
                }
                else
                {
                    dcd.ep0_stage = EP0_STAGE_DATA_IN;
                    USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_SVCRPR;

                    epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);
                    edpt->xfer.transferred = 0U;
                }

                dcd_event_setup_received(BOARD_TUD_RHPORT, &dcd.control_request.bmRequestType, true);
            }
        }
        break;

        case EP0_STAGE_DATA_IN:
        {
            /* Data IN stage */

            /* The data IN packet has been sent to the host */
            epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

            if ((edpt->xfer.transferred == dcd.control_request.wLength) ||
                (edpt->xfer.last_packet_size < edpt->max_packet_size))
            {
                /* The last data packet */
                dcd.ep0_stage = EP0_STAGE_STATUS_OUT;
            }

            /* Notify stack about the end of the data transaction */
            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.last_packet_size, XFER_RESULT_SUCCESS, true);
        }
        break;

        case EP0_STAGE_DATA_OUT:
        {
            /* Data OUT stage */
            if (status & USB_CSR0L_PERIPH_RXPKTRDY)
            {
                /* Data OUT token has been received */
                epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);

#if defined (BE_U1000) && defined (USB_DMA)
                edpt->xfer.last_packet_size = 0U;
                if (edpt->xfer.buffer != NULL)
                {
                    edpt->xfer.last_packet_size = USB->EPCSR[0].PERIPH.RXCOUNT;

                    USB->DMA.CH[edpt->dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.buffer;
                    USB->DMA.CH[edpt->dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
                    USB->DMA.CH[edpt->dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                                         USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                         USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
                }
#else
                edpt->xfer.last_packet_size = 0U;
                if (edpt->xfer.buffer != NULL)
                {
                    edpt->xfer.last_packet_size = rx_fifo_read(0U, edpt->xfer.buffer);
                    edpt->xfer.transferred += edpt->xfer.last_packet_size;
                }

                if ((edpt->xfer.transferred == dcd.control_request.wLength) ||
                    (edpt->xfer.last_packet_size < edpt->max_packet_size))
                {
                    /* The last data packet */
                    dcd.ep0_stage = EP0_STAGE_STATUS_IN;
                    USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_SVCRPR | USB_CSR0L_PERIPH_DATAEND);
                }

                /* Notify stack about the end of the data transaction */
                dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.last_packet_size, XFER_RESULT_SUCCESS, true);
#endif /* defined (BE_U1000) && defined (USB_DMA) */ 
            }
        }
        break;

        case EP0_STAGE_STATUS_IN:
        {
            /* Status IN stage */
            epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

            /* Notify stack about the status stage of control transfer */
            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, 0U, XFER_RESULT_SUCCESS, true);

            /* Set EP0 state to IDLE */
            dcd.ep0_stage = EP0_STAGE_IDLE;
        }
        break;

        case EP0_STAGE_STATUS_OUT:
        {
            /* Status OUT stage */
            epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);

            /* Notify stack about the status stage of control transfer */
            dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, 0U, XFER_RESULT_SUCCESS, true);

            /* Set EP0 state to IDLE */
            dcd.ep0_stage = EP0_STAGE_IDLE;
        }
        break;

        case EP0_STAGE_STATUS_SET_ADDRESS:
        {
            /* Status IN stage, handle Set Address request */

            /* Control transfer is complete, the device address can be changed */
            USB->COMMON.FADDR = dcd.dev_addr & USB_FADDR_FUNC;

            /* Set EP0 state to IDLE */
            dcd.ep0_stage = EP0_STAGE_IDLE;
        }
        break;

        default:
        {
            /* Set EP0 state to IDLE */
            dcd.ep0_stage = EP0_STAGE_IDLE;
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

#ifdef USB_BW_TEST_RX
        /* USB RX bandwidth test */
#if defined (BE_U1000) && defined (USB_DMA)
        edpt->xfer.last_packet_size = USB->EPCSR[epnum].PERIPH.RXCOUNT;

        USB->DMA.CH[edpt->dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.buffer;
        USB->DMA.CH[edpt->dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
        USB->DMA.CH[edpt->dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                             USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                             USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
#else
        edpt->xfer.last_packet_size = rx_fifo_read(epnum, edpt->xfer.buffer);   /* Read data from FIFO */
        USB->EPCSR[epnum].PERIPH.RXCSRL &= ~USB_RXCSRL_PERIPH_RXPKTRDY;         /* Clear RxPktRdy flag */
#endif /* defined (BE_U1000) && defined (USB_DMA) */

#else

#if defined (BE_U1000) && defined (USB_DMA)
        /* Configure DMA transfer */
        edpt->xfer.last_packet_size = USB->EPCSR[epnum].PERIPH.RXCOUNT;

        USB->DMA.CH[edpt->dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.buffer;
        USB->DMA.CH[edpt->dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
        USB->DMA.CH[edpt->dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                             USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                             USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
#else
        /* Copy data from FIFO */
        edpt->xfer.last_packet_size = rx_fifo_read(epnum, edpt->xfer.buffer);

        /* RxPktRdy flag will be cleared later in dcd_edpt_xfer() after the next
           data buffer is allocated. Until then the function will respond with NAK. */

        /* Notify USB stack about the data received */
        dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.last_packet_size, XFER_RESULT_SUCCESS, true);
#endif /* defined (BE_U1000) && defined (USB_DMA) */

#endif /* USB_BW_TEST_RX */
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
#if defined (USB_BW_TEST_RX)
        /* USB RX bandwidth test */
#elif defined (USB_BW_TEST_TX)
        /* USB TX bandwidth test */
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_IN);

#if defined (BE_U1000) && defined (USB_DMA)
        /* Configure DMA transfer */
        USB->DMA.CH[edpt->dma_ch].DMA_ADDR = (uint32_t)edpt->xfer.buffer;
        USB->DMA.CH[edpt->dma_ch].DMA_COUNT = edpt->xfer.last_packet_size;
        USB->DMA.CH[edpt->dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                             USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                             USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;

#if defined(USB_DPKTBUF)
        /* Disable the TX endpoint interrupt until DMA transfer is complete */
        USB->COMMON.INTRTXE &= ~(0x1U << epnum);
#endif /* USB_DPKTBUF */

#else
        tx_fifo_write(epnum, edpt->xfer.buffer, edpt->xfer.last_packet_size);
        USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
#endif /* defined (BE_U1000) && defined (USB_DMA) */

#else
        epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_IN);

        /* The data packet has been sent, notify USB stack */
        dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.last_packet_size, XFER_RESULT_SUCCESS, true);
#endif /* USB_BW_TEST_RX */
    }
}

#if !defined (BE_U1000) || !defined (USB_DMA)

/**
 * @brief Writes the endpoint FIFO.
 * @param epnum The endpoint number.
 * @param buffer The pointer to data buffer.
 * @param count The number of bytes to write.
 */
void tx_fifo_write(uint8_t epnum, uint8_t const * buffer, size_t count)
{
    if (buffer != NULL)
    {
        /* The endpoint buffer alignment is defined by CFG_TUSB_MEM_ALIGN macro 
        in tusb_option.h. Make sure the alignment size is 4 bytes for optimal
        performance. */
        uint32_t *buffer_word = (uint32_t*)buffer;

        volatile uint8_t * fifo_reg = (volatile uint8_t *)(&USB->FIFO[epnum]);
        volatile uint32_t * fifo_reg_word = (volatile uint32_t *)(&USB->FIFO[epnum]);

        size_t i = 0U;

        /* Write FIFO by word whenever possible */
        size_t count_word = count / sizeof(uint32_t);
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
 * @return The number of bytes have been read.
 */
uint16_t rx_fifo_read(uint8_t epnum, uint8_t * buffer)
{
    uint16_t count = USB->EPCSR[epnum].PERIPH.RXCOUNT;

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
        uint16_t count_word = count / sizeof(uint32_t);

        while (i < count_word)
        {
            buffer_word[i] = *fifo_reg_word;
            i++;
        }

        i *= sizeof(uint32_t);

        /* Read the rest of data by byte */
        while (i < count)
        {
            buffer[i] = *fifo_reg;
            i++;
        }
    }
    else
    {
        count = 0U;
    }

    return count;
}

#endif /* !defined (BE_U1000) || !defined (USB_DMA) */

void __attribute__ ((interrupt)) USB_IRQHandler(void)
{
    dcd_int_handler(BOARD_TUD_RHPORT);
}

#if defined (BE_U1000) && defined (USB_DMA)

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
                switch (dcd.ep0_stage)
                {
                    case EP0_STAGE_DATA_IN:
                    {
                        /* Data IN stage */
                        epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_IN);

                        /* On an input request, a device must never return more data
                            than is indicated by the wLength value; it may return less */
                        if ((edpt->xfer.transferred == dcd.control_request.wLength) ||
                            (edpt->xfer.last_packet_size < edpt->max_packet_size))
                        {
                            /* The last data packet */
                            USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_TXPKTRDY | USB_CSR0L_PERIPH_DATAEND);
                        }
                        else
                        {
                            /* Not the last data packet */
                            USB->EPCSR[0].PERIPH.CSR0L |= USB_CSR0L_PERIPH_TXPKTRDY;
                        }
                    }
                    break;

                    case EP0_STAGE_DATA_OUT:
                    {
                        /* Data OUT stage */
                        epn_ctrl_t * edpt = EP_CTRL_BASE(0, TUSB_DIR_OUT);

                        if ((edpt->xfer.transferred == dcd.control_request.wLength) ||
                            (edpt->xfer.last_packet_size < edpt->max_packet_size))
                        {
                            /* The last data packet */
                            dcd.ep0_stage = EP0_STAGE_STATUS_IN;
                            USB->EPCSR[0].PERIPH.CSR0L |= (USB_CSR0L_PERIPH_SVCRPR | USB_CSR0L_PERIPH_DATAEND);
                        }

                        /* Notify stack about the end of the data transaction */
                        dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.last_packet_size, XFER_RESULT_SUCCESS, true);
                    }
                    break;

                    default:
                    {
                        /* No action */
                    }
                    break;
                }
            }
            else if ((i % 2) != 0U)
            {
                /* RX endpoint */
                uint8_t epnum = ((i + 1U) >> 1);

#ifdef USB_BW_TEST_RX
                /* Clear RxPktRdy flag */
                USB->EPCSR[epnum].PERIPH.RXCSRL &= ~USB_RXCSRL_PERIPH_RXPKTRDY;
#else
                epn_ctrl_t * edpt = EP_CTRL_BASE(epnum, TUSB_DIR_OUT);
                
                /* Notify USB stack about the data received */
                dcd_event_xfer_complete(BOARD_TUD_RHPORT, edpt->ep_addr, edpt->xfer.last_packet_size, XFER_RESULT_SUCCESS, true);
#endif /* USB_BW_TEST_RX */
            }
            else
            {
                /* TX endpoint */
#ifdef USB_BW_TEST_RX
                /* USB RX bandwidth test */
#elif defined (USB_BW_TEST_TX)
                /* USB TX bandwidth test */
                uint8_t epnum = ((i + 1U) >> 1);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;

#if defined(USB_DPKTBUF)
                /* Re-enable the TX endpoint interrupt */
                USB->COMMON.INTRTXE |= (0x1U << epnum);
#endif /* USB_DPKTBUF */
   
#else
                uint8_t epnum = ((i + 1U) >> 1);
                USB->EPCSR[epnum].PERIPH.TXCSRL |= USB_TXCSRL_PERIPH_TXPKTRDY;
#endif /* USB_BW_TEST_RX */
            }
        }

        dmaInt >>= 1;
        i++;
    } 
}

#endif /* defined (BE_U1000) && defined (USB_DMA) */

#endif
