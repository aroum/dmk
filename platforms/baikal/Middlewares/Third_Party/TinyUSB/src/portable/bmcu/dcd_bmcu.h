/**
 * *****************************************************************************
 *  @file       dcd_bmcu.h
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB Device driver header file
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

#ifndef __DCD_BMCU_H
#define __DCD_BMCU_H

#if defined (BE_U1000)

#ifndef USBD_USE_DMA
#define USBD_USE_DMA                (1U)    /*!< Use USB DMA */

                                            /* NOTE 1: The amount of USB DMA channels is limited. Check that there is
                                                       enough DMA channels for the device being implemented at compile
                                                       time.
                                            */

                                            /* NOTE 2: DMA channel numbers are assigned as follows:
                                                       Endpoint 0: DMA channel 0 is used.
                                                       Rx endpoints: [DMA channel number] = [Endpoint number] * 2 - 1
                                                       Tx endpoints: [DMA channel number] = [Endpoint number] * 2
                                            */

                                            /* NOTE 3: The endpoints with the lower index will benefit from DMA usage, while
                                                       the endpoints with the higher index will operate in register mode.
                                            */
#endif /* USBD_USE_DMA */

#endif /* BE_U1000 */

#ifndef USBD_EP_MAX
#define USBD_EP_MAX                 (6U)    /*!< The maximum number of endpoints (control and non-control) */
#endif /* USBD_EP_MAX */

#if defined(USBD_SPEED_TEST)
#define USBD_DPKTBUF                        /*!< Use double packet buffering */
#endif /* USBD_SPEED_TEST */

#endif /* __DCD_BMCU_H */
