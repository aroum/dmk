/**
 * *****************************************************************************
 *  @file       hcd_bmcu.h
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB Host driver header file
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

#ifndef __HCD_BMCU_H
#define __HCD_BMCU_H

#include <stdint.h>
#include <stdbool.h>

#if defined (BE_U1000)

#ifndef USBH_USE_DMA
#define USBH_USE_DMA                (1U)    /*!< Use USB DMA */

                                            /* NOTE 1: take in account that the number of DMA channels is limited.
                                                       A DMA channel is assigned to a Rx or Tx endpoint. It should
                                                       suffice for a single device connected, but not for a hub.
                                            */

                                            /* NOTE 2: DMA channels are assigned to endpoints in the order of creation
                                                       up to the maximal number. DMA channel 0 is reserved for the EP0.
                                            */

                                            /* NOTE 3: The endpoints that do not have a DMA channel assigned to it
                                                       will operate in register mode.
                                            */
#endif /* USBH_USE_DMA */

#endif /* BE_U1000 */

#ifndef USBH_EP_MAX
#define USBH_EP_MAX                 (6U)    /*!< The maximum number of endpoints (control and non-control) */
#endif /* USBH_EP_MAX */

#if defined(USBH_SPEED_TEST)
#define USBH_DPKTBUF                        /*!< Use double packet buffering */
#endif /* USBH_SPEED_TEST */

#if defined (USBH_SPEED_TEST)

/** @brief Initializes the USB Host speed test. */
void hcd_speed_test_init(void);

/** @brief Sets the number of bytes to send during the USB Host speed test. */
void hcd_speed_test_set_data_lim(uint64_t size);

/** @brief Starts the USB Host speed test. */
void hcd_speed_test_start(void);

/** @brief Stops the USB Host speed test. */
void hcd_speed_test_stop(void);

/** @brief Checks, if the USB Host speed test is running. */
bool hcd_speed_test_is_running(void);

/** @brief Returns the USB Host speed test start time. */
uint64_t hcd_speed_test_get_start_time(void);

/** @brief Returns the USB Host speed test stop time. */
uint64_t hcd_speed_test_get_stop_time(void);

/** @brief Returns the number of bytes sent during the USB Host speed test. */
uint64_t hcd_speed_test_get_bytes_sent(void);

/** @brief Prints an unsigned 64-bit number. */
void hcd_speed_test_print64(uint64_t value);

/** @brief Prints the USB Host speed test results. */
void hcd_speed_test_print_stats(void);

#endif /* defined (USBH_SPEED_TEST) */

#endif /* __HCD_BMCU_H */
