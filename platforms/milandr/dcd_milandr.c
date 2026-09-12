/**
 * @file dcd_milandr.c
 * @brief TinyUSB Device Controller Driver (DCD) for Milandr K1986BE92QI (MDR32F9Q2I).
 */

#include "tusb_option.h"

#if CFG_TUD_ENABLED && defined(MCU_milandr)

#include "MDR32FxQI_config.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_usb.h"
#include "device/dcd.h"
#include <string.h>

#define MILANDR_NUM_EP 4

typedef struct {
    uint8_t *buffer;
    uint16_t total_len;
    uint16_t transferred_len;
    uint16_t max_packet_size;
    uint8_t data_toggle; // 0 = DATA0, 1 = DATA1
    bool is_open;
    bool is_in;
} milandr_ep_t;

static milandr_ep_t ep_state[MILANDR_NUM_EP];

//--------------------------------------------------------------------+
// Controller API
//--------------------------------------------------------------------+

bool dcd_init(uint8_t rhport, const tusb_rhport_init_t *rh_init) {
    (void)rhport;
    (void)rh_init;

    // Enable clock to USB periphery
    RST_CLK_PCLKcmd(RST_CLK_PCLK_USB, ENABLE);

    // Setup USB PLL: HSE 8MHz / 1 * 6 = 48MHz
    USB_Clock_TypeDef usb_clk;
    usb_clk.USB_USBC1_Source = USB_C1HSEdiv1;
    usb_clk.USB_PLLUSBMUL = USB_PLLUSBMUL6;
    USB_BRGInit(&usb_clk);

    // Reset USB controller core
    USB_Reset();

    // Set Device mode
    USB_SetHSCR(USB_HSCR_HOST_MODE_Device);

    // Set speed, polarity and enable slave controller (SCGEN)
    USB_SetSC(USB_SC_SCFSR_12Mb | USB_SC_SCFSP_Full | USB_SC_SCGEN_Set);

    // Power on RX and TX transceivers
    USB_SetHSCR(USB_HSCR_EN_RX_Set | USB_HSCR_EN_TX_Set);

    // Initialize endpoint contexts and reset FIFOs
    for (int i = 0; i < MILANDR_NUM_EP; i++) {
        memset(&ep_state[i], 0, sizeof(milandr_ep_t));
        USB_SetSEPxCTRL((USB_EP_TypeDef)i, USB_SEPx_CTRL_EPEN_Disable | USB_SEPx_CTRL_EPRDY_NotReady);
        USB_SetSEPxRXFC((USB_EP_TypeDef)i, 1);
        USB_SetSEPxTXFDC((USB_EP_TypeDef)i, 1);
    }

    // Set initial device address to 0
    USB_SetSA(0);

    // Configure interrupt mask
    USB_SetSIM(USB_SIM_SCTDONEIE_Set | USB_SIM_SCRESETEVIE_Set | USB_SIM_SCRESUMEIE_Set);

    // Clear any pending interrupt status flags
    USB_SetSIS(USB_SIS_Msk);

    // Connect pull-up resistor to D+ line for host enumeration
    dcd_connect(rhport);

    // Configure interrupt in NVIC
    NVIC_SetPriority(USB_IRQn, 6);
    NVIC_EnableIRQ(USB_IRQn);

    return true;
}

bool dcd_deinit(uint8_t rhport) {
    (void)rhport;
    dcd_disconnect(rhport);
    NVIC_DisableIRQ(USB_IRQn);
    return true;
}

void dcd_int_enable(uint8_t rhport) {
    (void)rhport;
    NVIC_EnableIRQ(USB_IRQn);
}

void dcd_int_disable(uint8_t rhport) {
    (void)rhport;
    NVIC_DisableIRQ(USB_IRQn);
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr) {
    (void)dev_addr;
    // USB spec: Respond with 0-length status packet on EP0 IN using old address.
    // Address change takes effect after status phase in dcd_edpt0_status_complete().
    dcd_edpt_xfer(rhport, 0x80, NULL, 0);
}

void dcd_edpt0_status_complete(uint8_t rhport, tusb_control_request_t const *request) {
    (void)rhport;
    if (request->bRequest == TUSB_REQ_SET_ADDRESS) {
        uint8_t const dev_addr = (uint8_t)request->wValue;
        USB_SetSA(dev_addr);
    }
}

void dcd_remote_wakeup(uint8_t rhport) {
    (void)rhport;
}

void dcd_connect(uint8_t rhport) {
    (void)rhport;
    USB_SetHSCR(USB_HSCR_DP_PULLUP_Set);
}

void dcd_disconnect(uint8_t rhport) {
    (void)rhport;
    USB_SetHSCR(USB_HSCR_DP_PULLUP_Reset);
}

void dcd_sof_enable(uint8_t rhport, bool en) {
    (void)rhport;
    (void)en;
}

//--------------------------------------------------------------------+
// Endpoint API
//--------------------------------------------------------------------+

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const *desc_ep) {
    (void)rhport;
    uint8_t const ep_addr = desc_ep->bEndpointAddress;
    uint8_t const epnum = tu_edpt_number(ep_addr);
    bool const is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    if (epnum >= MILANDR_NUM_EP)
        return false;

    milandr_ep_t *ep = &ep_state[epnum];
    ep->max_packet_size = tu_edpt_packet_size(desc_ep);
    ep->is_in = is_in;
    ep->is_open = true;
    ep->data_toggle = 0; // DATA0
    ep->buffer = NULL;
    ep->total_len = 0;
    ep->transferred_len = 0;

    // Reset FIFOs
    USB_SetSEPxRXFC((USB_EP_TypeDef)epnum, 1);
    USB_SetSEPxTXFDC((USB_EP_TypeDef)epnum, 1);

    // Configure and enable endpoint
    uint32_t ctrl = USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPRDY_NotReady | USB_SEPx_CTRL_EPDATASEQ_Data0 |
                    USB_SEPx_CTRL_EPSSTALL_NotReply;

    USB_SetSEPxCTRL((USB_EP_TypeDef)epnum, ctrl);

    return true;
}

void dcd_edpt_close_all(uint8_t rhport) {
    (void)rhport;
    for (uint8_t i = 1; i < MILANDR_NUM_EP; i++) {
        USB_SetSEPxCTRL((USB_EP_TypeDef)i, USB_SEPx_CTRL_EPEN_Disable | USB_SEPx_CTRL_EPRDY_NotReady);
        USB_SetSEPxRXFC((USB_EP_TypeDef)i, 1);
        USB_SetSEPxTXFDC((USB_EP_TypeDef)i, 1);
        ep_state[i].is_open = false;
    }
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes) {
    (void)rhport;
    uint8_t const epnum = tu_edpt_number(ep_addr);
    bool const is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    if (epnum >= MILANDR_NUM_EP)
        return false;

    milandr_ep_t *ep = &ep_state[epnum];
    ep->buffer = buffer;
    ep->total_len = total_bytes;
    ep->transferred_len = 0;
    ep->is_in = is_in;

    if (is_in) {
        // Calculate packet size to load into TX FIFO
        uint16_t packet_size = tu_min16(total_bytes, ep->max_packet_size);

        // Reset TX FIFO
        USB_SetSEPxTXFDC((USB_EP_TypeDef)epnum, 1);

        // Load data into TX FIFO
        for (uint16_t i = 0; i < packet_size; i++) {
            USB_SetSEPxTXFD((USB_EP_TypeDef)epnum, buffer[i]);
        }

        uint32_t data_seq = (ep->data_toggle == 1) ? USB_SEPx_CTRL_EPDATASEQ_Data1 : USB_SEPx_CTRL_EPDATASEQ_Data0;

        // Clear SCTDONE in SIS before arming EPRDY
        USB_SetSIS(USB_SIS_Msk);

        // Arm endpoint for transmission
        USB_SetSEPxCTRL((USB_EP_TypeDef)epnum, USB_SEPx_CTRL_EPEN_Enable | data_seq | USB_SEPx_CTRL_EPRDY_Ready);
    } else {
        // Clear RX FIFO
        USB_SetSEPxRXFC((USB_EP_TypeDef)epnum, 1);

        uint32_t data_seq = (ep->data_toggle == 1) ? USB_SEPx_CTRL_EPDATASEQ_Data1 : USB_SEPx_CTRL_EPDATASEQ_Data0;

        // Clear SCTDONE in SIS before arming EPRDY
        USB_SetSIS(USB_SIS_Msk);

        // Arm endpoint for reception
        USB_SetSEPxCTRL((USB_EP_TypeDef)epnum, USB_SEPx_CTRL_EPEN_Enable | data_seq | USB_SEPx_CTRL_EPRDY_Ready);
    }

    return true;
}

void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr) {
    (void)rhport;
    uint8_t const epnum = tu_edpt_number(ep_addr);
    if (epnum < MILANDR_NUM_EP) {
        USB_SetSEPxCTRL((USB_EP_TypeDef)epnum, USB_SEPx_CTRL_EPSSTALL_Reply);
    }
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr) {
    (void)rhport;
    uint8_t const epnum = tu_edpt_number(ep_addr);
    if (epnum < MILANDR_NUM_EP) {
        ep_state[epnum].data_toggle = 0;
        USB_SetSEPxCTRL((USB_EP_TypeDef)epnum, USB_SEPx_CTRL_EPSSTALL_NotReply | USB_SEPx_CTRL_EPDATASEQ_Data0);
    }
}

//--------------------------------------------------------------------+
// Interrupt Handler
//--------------------------------------------------------------------+

void dcd_int_handler(uint8_t rhport) {
    uint32_t sis = USB_GetSIS();

    // 1. Bus Reset Event
    if (sis & USB_SIS_SCRESETEV) {
        USB_SetSIS(USB_SIS_SCRESETEV);
        USB_SetSA(0);

        ep_state[0].data_toggle = 0;
        ep_state[0].max_packet_size = 64;
        ep_state[0].is_open = true;

        // Reset and prepare EP0 for incoming SETUP packet
        USB_SetSEPxRXFC(USB_EP0, 1);
        USB_SetSEPxCTRL(USB_EP0, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data0 | USB_SEPx_CTRL_EPRDY_Ready);

        dcd_event_bus_reset(rhport, TUSB_SPEED_FULL, true);
        return;
    }

    // 2. Bus Resume Event
    if (sis & USB_SIS_SCRESUME) {
        USB_SetSIS(USB_SIS_SCRESUME);
        dcd_event_bus_signal(rhport, DCD_EVENT_RESUME, true);
    }

    // 3. Transaction Done Event
    if (sis & USB_SIS_SCTDONE) {
        USB_SetSIS(USB_SIS_SCTDONE);

        // --- Handle Control Endpoint EP0 ---
        uint32_t ep0_ctrl = USB_GetSEPxCTRL(USB_EP0);
        if (!(ep0_ctrl & USB_SEP_CTRL_EPRDY)) {
            uint32_t ts = USB_GetSEPxTS(USB_EP0) & USB_SEPx_TS_SCTTYPE_Msk;

            if (ts == USB_SEPx_TS_SCTTYPE_Setup) {
                // Read 8-byte SETUP packet
                uint8_t setup[8];
                for (int i = 0; i < 8; i++) {
                    setup[i] = (uint8_t)USB_GetSEPxRXFD(USB_EP0);
                }
                USB_SetSEPxRXFC(USB_EP0, 1);

                // In USB spec, receiving SETUP resets toggle for data phase to DATA1
                ep_state[0].data_toggle = 1;
                ep_state[0].transferred_len = 0;
                ep_state[0].total_len = 0;
                ep_state[0].buffer = NULL;

                dcd_event_setup_received(rhport, setup, true);
            } else if (ts == USB_SEPx_TS_SCTTYPE_In) {
                // Control IN transaction complete
                ep_state[0].data_toggle ^= 1;
                milandr_ep_t *ep = &ep_state[0];
                uint16_t packet_size = tu_min16(ep->total_len - ep->transferred_len, ep->max_packet_size);
                ep->transferred_len += packet_size;

                if (ep->transferred_len < ep->total_len) {
                    // Send next portion of control data
                    uint16_t next_size = tu_min16(ep->total_len - ep->transferred_len, ep->max_packet_size);
                    USB_SetSEPxTXFDC(USB_EP0, 1);
                    for (uint16_t i = 0; i < next_size; i++) {
                        USB_SetSEPxTXFD(USB_EP0, ep->buffer[ep->transferred_len + i]);
                    }
                    uint32_t data_seq =
                        (ep->data_toggle == 1) ? USB_SEPx_CTRL_EPDATASEQ_Data1 : USB_SEPx_CTRL_EPDATASEQ_Data0;
                    USB_SetSEPxCTRL(USB_EP0, USB_SEPx_CTRL_EPEN_Enable | data_seq | USB_SEPx_CTRL_EPRDY_Ready);
                } else {
                    dcd_event_xfer_complete(rhport, 0x80, ep->transferred_len, XFER_RESULT_SUCCESS, true);
                }
            } else if (ts == USB_SEPx_TS_SCTTYPE_Outdata) {
                // Control OUT transaction complete
                ep_state[0].data_toggle ^= 1;
                milandr_ep_t *ep = &ep_state[0];
                uint16_t count = (uint16_t)(USB_GetSEPxRXFDC(USB_EP0) & 0xFF);

                for (uint16_t i = 0; i < count; i++) {
                    uint8_t byte = (uint8_t)USB_GetSEPxRXFD(USB_EP0);
                    if (ep->buffer && (ep->transferred_len + i < ep->total_len)) {
                        ep->buffer[ep->transferred_len + i] = byte;
                    }
                }
                USB_SetSEPxRXFC(USB_EP0, 1);
                ep->transferred_len += count;

                dcd_event_xfer_complete(rhport, 0x00, ep->transferred_len, XFER_RESULT_SUCCESS, true);
            }
        }

        // --- Handle Non-Control Endpoints EP1..EP3 ---
        for (uint8_t epnum = 1; epnum < MILANDR_NUM_EP; epnum++) {
            milandr_ep_t *ep = &ep_state[epnum];
            if (!ep->is_open)
                continue;

            uint32_t ctrl = USB_GetSEPxCTRL((USB_EP_TypeDef)epnum);
            if (!(ctrl & USB_SEP_CTRL_EPRDY)) {
                uint32_t ts = USB_GetSEPxTS((USB_EP_TypeDef)epnum) & USB_SEPx_TS_SCTTYPE_Msk;
                uint32_t sts = USB_GetSEPxSTS((USB_EP_TypeDef)epnum);

                if (ep->is_in && ts == USB_SEPx_TS_SCTTYPE_In && (sts & USB_SEP_STS_SCACKRXED)) {
                    // IN packet was transmitted and ACKed by host
                    ep->data_toggle ^= 1;
                    uint16_t packet_size = tu_min16(ep->total_len - ep->transferred_len, ep->max_packet_size);
                    ep->transferred_len += packet_size;

                    if (ep->transferred_len < ep->total_len) {
                        // Send next packet
                        uint16_t next_size = tu_min16(ep->total_len - ep->transferred_len, ep->max_packet_size);
                        USB_SetSEPxTXFDC((USB_EP_TypeDef)epnum, 1);
                        for (uint16_t i = 0; i < next_size; i++) {
                            USB_SetSEPxTXFD((USB_EP_TypeDef)epnum, ep->buffer[ep->transferred_len + i]);
                        }
                        uint32_t data_seq =
                            (ep->data_toggle == 1) ? USB_SEPx_CTRL_EPDATASEQ_Data1 : USB_SEPx_CTRL_EPDATASEQ_Data0;
                        USB_SetSEPxCTRL((USB_EP_TypeDef)epnum,
                                        USB_SEPx_CTRL_EPEN_Enable | data_seq | USB_SEPx_CTRL_EPRDY_Ready);
                    } else {
                        // All requested bytes transmitted
                        dcd_event_xfer_complete(rhport, (uint8_t)(epnum | 0x80), ep->transferred_len,
                                                XFER_RESULT_SUCCESS, true);
                    }
                } else if (!ep->is_in && ts == USB_SEPx_TS_SCTTYPE_Outdata) {
                    // OUT packet was received from host
                    ep->data_toggle ^= 1;
                    uint16_t count = (uint16_t)(USB_GetSEPxRXFDC((USB_EP_TypeDef)epnum) & 0xFF);

                    for (uint16_t i = 0; i < count; i++) {
                        uint8_t byte = (uint8_t)USB_GetSEPxRXFD((USB_EP_TypeDef)epnum);
                        if (ep->buffer && (ep->transferred_len + i < ep->total_len)) {
                            ep->buffer[ep->transferred_len + i] = byte;
                        }
                    }
                    USB_SetSEPxRXFC((USB_EP_TypeDef)epnum, 1);
                    ep->transferred_len += count;

                    dcd_event_xfer_complete(rhport, epnum, ep->transferred_len, XFER_RESULT_SUCCESS, true);
                }
            }
        }
    }
}

#endif // CFG_TUD_ENABLED && defined(MCU_milandr)
