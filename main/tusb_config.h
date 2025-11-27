/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define CFG_TUD_ENABLED         (1)

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG        0
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_PICO
#endif

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

// Legacy RHPORT configuration
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT        (0)
#endif
// end legacy RHPORT

//------------------------
// DEVICE CONFIGURATION //
//------------------------

// Enable HID
#define CFG_TUD_HID             (0)
// set HID endpoint buff size
#define CFG_TUD_HID_EP_BUFSIZE  (64)

// Enable vendr
#define CFG_TUD_VENDOR          (1)

// Enable 2 CDC classes
#define CFG_TUD_CDC             (1)
// Set CDC FIFO buffer sizes
#define CFG_TUD_CDC_RX_BUFSIZE  (1024)
#define CFG_TUD_CDC_TX_BUFSIZE  (1024)
#define CFG_TUD_CDC_EP_BUFSIZE  (256)

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE  (64)
#endif

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
