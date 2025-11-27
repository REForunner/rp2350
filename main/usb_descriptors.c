/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tusb.h>
#include "serial.h"
#include "dap/DAP_config.h"
#include "dap/DAP.h"

// set some example Vendor and Product ID
// the board will use to identify at the host
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define CDC_EXAMPLE_VID     0x2E8A
// use _PID_MAP to generate unique PID for each interface
#define CDC_EXAMPLE_PID     ( 0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )
// set USB 2.0
#if CFG_TUD_HID
    #define CDC_EXAMPLE_BCD     0x0200
#endif
// set USB 2.1
#if CFG_TUD_VENDOR
    #define CDC_EXAMPLE_BCD     0x0210
#endif
//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// defines a descriptor that will be communicated to the host
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = CDC_EXAMPLE_BCD,

    .bDeviceClass = TUSB_CLASS_MISC, // CDC is a subclass of misc
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, // CDC uses common subclass
    .bDeviceProtocol = MISC_PROTOCOL_IAD, // CDC uses IAD

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE, // 64 bytes

    .idVendor = CDC_EXAMPLE_VID,
    .idProduct = CDC_EXAMPLE_PID,
    .bcdDevice = 0x0100, // Device release number

    .iManufacturer = 0x01, // Index of manufacturer string
    .iProduct = 0x02, // Index of product string
    .iSerialNumber = 0x03, // Index of serial number string

    .bNumConfigurations = 0x01 // 1 configuration
};

// called when host requests to get device descriptor
uint8_t const *tud_descriptor_device_cb(void);

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
#if CFG_TUD_HID
uint8_t const desc_hid_report[] = {
        TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE)
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return desc_hid_report;
}
#endif
//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
// String descriptors referenced with .i... in the descriptor tables

enum {
    STRID_LANGID = 0,   // 0: supported language ID
    STRID_MANUFACTURER, // 1: Manufacturer
    STRID_PRODUCT,      // 2: Product
    STRID_SERIAL,       // 3: Serials
#if (CFG_TUD_CDC == 1)
    STRID_CDC_0,        // 4: CDC Interface 0
#elif (CFG_TUD_CDC == 2)
    STRID_CDC_0,        // 4: CDC Interface 0
    STRID_CDC_1,        // 5: CDC Interface 1
#endif
#if CFG_TUD_HID
    STRID_HID,          // 6: HID Interface
#else
    STRID_VENDOR,       // 6: Vendor Interface
#endif
};

enum {
#if CFG_TUD_HID
    ITF_NUM_HID = 0,
#else
    TIF_NUM_VENDOR = 0, // Old versions of Keil MDK only look at interface 0 ???
#endif
#if (CFG_TUD_CDC == 1)
    ITF_NUM_CDC_0,
    ITF_NUM_CDC_0_DATA,
#elif (CFG_TUD_CDC == 2)
    ITF_NUM_CDC_0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,
    ITF_NUM_CDC_1_DATA,
#endif
    ITF_NUM_TOTAL
};

#if CFG_TUD_HID
// total length of configuration descriptor
#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#else
// total length of configuration descriptor
#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN)
#endif

// define endpoint numbers
#if (CFG_TUD_CDC == 1)
    #define EPNUM_CDC_0_NOTIF   0x81 // notification endpoint for CDC 0
    #define EPNUM_CDC_0_OUT     0x02 // out endpoint for CDC 0
    #define EPNUM_CDC_0_IN      0x82 // in endpoint for CDC 0
#elif (CFG_TUD_CDC == 2)
    #define EPNUM_CDC_0_NOTIF   0x81 // notification endpoint for CDC 0
    #define EPNUM_CDC_0_OUT     0x02 // out endpoint for CDC 0
    #define EPNUM_CDC_0_IN      0x82 // in endpoint for CDC 0

    #define EPNUM_CDC_1_NOTIF   0x84 // notification endpoint for CDC 1
    #define EPNUM_CDC_1_OUT     0x05 // out endpoint for CDC 1
    #define EPNUM_CDC_1_IN      0x85 // in endpoint for CDC 1
#endif

#if (CFG_TUD_HID || CFG_TUD_VENDOR)
    #define EPNUM_HID_OUT       0x06
    #define EPNUM_HID_IN        0x86
#endif

// configure descriptor (for 2 CDC interfaces)
uint8_t const desc_configuration[] = {
    // config descriptor | how much power in mA, count of interfaces, ...
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, STRID_LANGID, CONFIG_TOTAL_LEN, 0x80, 100),

#if CFG_TUD_HID
    // HID Interface
    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, STRID_HID, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID_IN, EPNUM_HID_OUT, CFG_TUD_HID_EP_BUFSIZE, 1),
#else
    TUD_VENDOR_DESCRIPTOR(TIF_NUM_VENDOR, STRID_VENDOR, EPNUM_HID_OUT, EPNUM_HID_IN, 64),
#endif
#if (CFG_TUD_CDC == 1)
    // CDC 0: Communication Interface - TODO: get 64 from tusb_config.h
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, STRID_CDC_0, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64),
    // CDC 0: Data Interface
    //TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0_DATA, 4, 0x01, 0x02),
#elif (CFG_TUD_CDC == 2)
    // CDC 0: Communication Interface - TODO: get 64 from tusb_config.h
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, STRID_CDC_0, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64),
    // CDC 0: Data Interface
    //TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0_DATA, 4, 0x01, 0x02),

    // CDC 1: Communication Interface - TODO: get 64 from tusb_config.h
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, STRID_CDC_1, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_OUT, EPNUM_CDC_1_IN, 64),
    // CDC 1: Data Interface
    //TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1_DATA, 4, 0x03, 0x04),
#endif
};

// called when host requests to get configuration descriptor
uint8_t const * tud_descriptor_configuration_cb(uint8_t index);

// more device descriptor this time the qualifier
tusb_desc_device_qualifier_t const desc_device_qualifier = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = CDC_EXAMPLE_BCD,

    .bDeviceClass = TUSB_CLASS_CDC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .bNumConfigurations = 0x01,
    .bReserved = 0x00
};

// called when host requests to get device qualifier descriptor
uint8_t const* tud_descriptor_device_qualifier_cb(void);

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    // switched because board is little endian
    (const char[]) { 0x09, 0x04 },  // 0: supported language is English (0x0409)
    "Raspberry Pi",                 // 1: Manufacturer
    "Pico (2)",                     // 2: Product
    NULL,                           // 3: Serials (null so it uses unique ID if available)
#if (CFG_TUD_CDC == 1)
    "Pico SDK stdio",               // 4: CDC Interface 0
#elif (CFG_TUD_CDC == 2)
    "Pico SDK stdio",               // 4: CDC Interface 0
    "Custom CDC",                   // 5: CDC Interface 1
#endif
#if CFG_TUD_HID
    "#HID CMSIS-DAP v" DAP_FW_VER,  // 6: HID Interface
#else
    "#WinUSB CMSIS-DAP v" DAP_FW_VER, // 6: Interface descriptor for Bulk transport
#endif
    "RPiReset"                      // 7: Reset Interface
};

// buffer to hold the string descriptor during the request | plus 1 for the null terminator
static uint16_t _desc_str[32 + 1];

// called when host request to get string descriptor
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid);

// --------------------------------------------------------------------+
// IMPLEMENTATION
// --------------------------------------------------------------------+

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
    return (uint8_t const *)&desc_device_qualifier;
}

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    // avoid unused parameter warning and keep function signature consistent
    (void)index;

    return desc_configuration;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    // TODO: check lang id
    (void) langid;
    size_t char_count;

    // Determine which string descriptor to return
    switch (index) {
        case STRID_LANGID:
            memcpy(&_desc_str[1], string_desc_arr[STRID_LANGID], 2);
            char_count = 1;
            break;

        case STRID_SERIAL:
            // try to read the serial from the board
            char_count = usb_get_serial(_desc_str + 1, 32);
            break;

        default:
            // COPYRIGHT NOTE: Based on TinyUSB example
            // Windows wants utf16le

            // Determine which string descriptor to return
            if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) {
                return NULL;
            }

            // Copy string descriptor into _desc_str
            const char *str = string_desc_arr[index];

            char_count = strlen(str);
            size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
            // Cap at max char
            if (char_count > max_count) {
                char_count = max_count;
            }

            // Convert ASCII string into UTF-16
            for (size_t i = 0; i < char_count; i++) {
                _desc_str[1 + i] = str[i];
            }
            break;
    }

    // First byte is the length (including header), second byte is string type
    _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (char_count * 2 + 2));

    return _desc_str;
}

/* [incoherent gibbering to make Windows happy] */

//--------------------------------------------------------------------+
// BOS Descriptor
//--------------------------------------------------------------------+

/* Microsoft OS 2.0 registry property descriptor
Per MS requirements https://msdn.microsoft.com/en-us/library/windows/hardware/hh450799(v=vs.85).aspx
device should create DeviceInterfaceGUIDs. It can be done by driver and
in case of real PnP solution device should expose MS "Microsoft OS 2.0
registry property descriptor". Such descriptor can insert any record
into Windows registry per device/configuration/interface. In our case it
will insert "DeviceInterfaceGUIDs" multistring property.


https://developers.google.com/web/fundamentals/native-hardware/build-for-webusb/
(Section Microsoft OS compatibility descriptors)
*/
#define MS_OS_20_DESC_LEN  0xB2

#define BOS_TOTAL_LEN      (TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

uint8_t const desc_bos[] =
{
  // total length, number of device caps
  TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 1),

  // Microsoft OS 2.0 descriptor
  TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, 1)
};

uint8_t const desc_ms_os_20[] =
{
  // Set header: length, type, windows version, total length
  U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

  // Configuration subset header: length, type, configuration index, reserved, configuration total length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A),

  // Function Subset header: length, type, first interface, reserved, subset length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), 0/*TIF_NUM_VENDOR*/, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08),

  // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
  U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

  // MS OS 2.0 Registry property descriptor: length, type
  U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08-0x08-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
  U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
  'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
  U16_TO_U8S_LE(0x0050), // wPropertyDataLength
  // bPropertyData "{CDB3B5AD-293B-4663-AA36-1AAE46463776}" as a UTF-16 string (b doesn't mean bytes)
  '{', 0x00, 'C', 0x00, 'D', 0x00, 'B', 0x00, '3', 0x00, 'B', 0x00, '5', 0x00, 'A', 0x00, 'D', 0x00, '-', 0x00,
  '2', 0x00, '9', 0x00, '3', 0x00, 'B', 0x00, '-', 0x00, '4', 0x00, '6', 0x00, '6', 0x00, '3', 0x00, '-', 0x00,
  'A', 0x00, 'A', 0x00, '3', 0x00, '6', 0x00, '-', 0x00, '1', 0x00, 'A', 0x00, 'A', 0x00, 'E', 0x00, '4', 0x00,
  '6', 0x00, '4', 0x00, '6', 0x00, '3', 0x00, '7', 0x00, '7', 0x00, '6', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

TU_VERIFY_STATIC(sizeof(desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");

uint8_t const * tud_descriptor_bos_cb(void)
{
  return desc_bos;
}

//--------------------------------------------------------------------+
// USB VENDOR
//--------------------------------------------------------------------+
#if CFG_TUD_VENDOR

#define VENDOR_REQUEST_MICROSOFT    1

extern uint8_t const desc_ms_os_20[];

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
  // nothing to with DATA & ACK stage
  if (stage != CONTROL_STAGE_SETUP) return true;

  switch (request->bmRequestType_bit.type)
  {
    case TUSB_REQ_TYPE_VENDOR:
      switch (request->bRequest)
      {
        case VENDOR_REQUEST_MICROSOFT:
          if ( request->wIndex == 7 )
          {
            // Get Microsoft OS 2.0 compatible descriptor
            uint16_t total_len;
            memcpy(&total_len, desc_ms_os_20 + 8, 2);

            return tud_control_xfer(rhport, request, (void*) desc_ms_os_20, total_len);
          }
          else
          {
            return false;
          }

        default: break;
      }
    break;
    default: break;
  }

  // stall unknown request
  return false;
}
#endif