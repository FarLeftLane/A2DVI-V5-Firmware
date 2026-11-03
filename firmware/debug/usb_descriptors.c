/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#if PICO_USB
#warning PICO_USB FEATURE IS ENABLED! ************************************

#include <stdbool.h>            //  For bool
#include <pico/unique_id.h>
#include <tusb.h>
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "device/usbd_pvt.h"


// VENDOR sub-class for the reset interface
#define RESET_INTERFACE_SUBCLASS 0x00
// VENDOR protocol for the reset interface
#define RESET_INTERFACE_PROTOCOL 0x01

// reset to BOOTSEL
#define RESET_REQUEST_BOOTSEL 0x01
// regular flash boot
#define RESET_REQUEST_FLASH 0x02


#if 1       //  We seem to need this for Reset to work

#define USB_VID (0x2E8A) // Raspberry Pi
#define USB_PID (0x0009) // Raspberry Pi Pico SDK CDC

#else
/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID (0x4000 | \
                 _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) |  _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define USB_VID 0x6502
#endif

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
#if 1
    ITF_NUM_RESET,
#endif
    ITF_NUM_TOTAL
};

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82

#define EPNUM_MSC_OUT   0x03
#define EPNUM_MSC_IN    0x83

#define TUD_RPI_RESET_DESC_LEN  9

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + (CFG_TUD_CDC * TUD_CDC_DESC_LEN) \
                                               + (CFG_TUD_MSC * TUD_MSC_DESC_LEN) \
                                               + (CFG_TUD_VENDOR * TUD_RPI_RESET_DESC_LEN) )

//  RESET descriptor 
#define TUD_RPI_RESET_DESCRIPTOR(_itfnum, _stridx) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, RESET_INTERFACE_SUBCLASS, RESET_INTERFACE_PROTOCOL, _stridx,


// Full speed configuration
uint8_t const desc_fs_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

#if 1
    TUD_RPI_RESET_DESCRIPTOR(ITF_NUM_RESET, 5)          //  If CFG_TUD_MSC is zero the 6 needs to be a 5
#endif
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations
    return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// Buffer to hold flash ID
static char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

// Array of pointer to string descriptors
char const* string_desc_arr [] =
{
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "A2DVIV5",                  // 1: Manufacturer
    "A2DVIV5_PV",               // 2: Product
    serial,                     // 3: Serials, uses flash ID
    "A2DVIV5 CDC",              // 4: CDC Interface
    "Reset"                     // 5: RESET Interface
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;

    } else {
      // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
      // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

      if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
          return NULL;
      }

      if (index == 3 && !serial[0]) {
          pico_get_unique_board_id_string(serial, sizeof(serial));
      }

      const char* str = string_desc_arr[index];

      // Cap at max char
      chr_count = strlen(str);
      if (chr_count > 31) {
          chr_count = 31;
      }

      // Convert ASCII string into UTF-16
      for (uint8_t i = 0; i < chr_count; i++) {
          _desc_str[1 + i] = str[i];
      }
    }

    // First byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}


static uint8_t itf_num;

static void resetd_init(void) {
}

static void resetd_reset(uint8_t __unused rhport) {
    itf_num = 0;
}

static uint16_t resetd_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
              RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
              RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t);
    TU_VERIFY(max_len >= drv_len, 0);

    itf_num = itf_desc->bInterfaceNumber;
    return drv_len;
}

// Support for parameterized reset via vendor interface control request
static bool resetd_control_xfer_cb(uint8_t __unused rhport, uint8_t stage, tusb_control_request_t const * request) {
    // nothing to do with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP) return true;

    if (request->wIndex == itf_num) {

        if (request->bRequest == RESET_REQUEST_BOOTSEL) {
            //  Reboot into BOOTSEL
            // rom_reset_usb_boot_extra(-1, ((request->wValue & 0x7f) | (0u)), false);
            rom_reset_usb_boot_extra(-1, (0u), false);

            // does not return, otherwise we'd return true
        }
    }
    return false;
}

static bool resetd_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr, xfer_result_t __unused result, uint32_t __unused xferred_bytes) {
    return true;
}

static usbd_class_driver_t const _resetd_driver =
{
#if CFG_TUSB_DEBUG >= 2
    .name = "RESET",
#endif
    .init             = resetd_init,
    .reset            = resetd_reset,
    .open             = resetd_open,
    .control_xfer_cb  = resetd_control_xfer_cb,
    .xfer_cb          = resetd_xfer_cb,
    .sof              = NULL
};

// Implement callback to add our custom driver
usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &_resetd_driver;
}
#endif