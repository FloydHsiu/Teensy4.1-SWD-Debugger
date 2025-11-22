/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2019 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// #if F_CPU >= 20000000

#define USB_DESC_LIST_DEFINE
#include "cmsis_dap/usb_desc_cmsis_dap.h"
#ifdef NUM_ENDPOINTS
#include "avr/pgmspace.h"
#include "avr_functions.h"
#include "imxrt.h"
#include "usb_names.h"

// At very slow CPU speeds, the OCRAM just isn't fast enough for
// USB to work reliably.  But the precious/limited DTCM is.  So
// as an ugly workaround, undefine DMAMEM so all buffers which
// would normally be allocated in OCRAM are placed in DTCM.
#if defined(F_CPU) && F_CPU < 30000000
#undef DMAMEM
#endif

// USB Descriptors are binary data which the USB host reads to
// automatically detect a USB device's capabilities.  The format
// and meaning of every field is documented in numerous USB
// standards.  When working with USB descriptors, despite the
// complexity of the standards and poor writing quality in many
// of those documents, remember descriptors are nothing more
// than constant binary data that tells the USB host what the
// device can do.  Computers will load drivers based on this data.
// Those drivers then communicate on the endpoints specified by
// the descriptors.

// To configure a new combination of interfaces or make minor
// changes to existing configuration (eg, change the name or ID
// numbers), usually you would edit "usb_desc.h".  This file
// is meant to be configured by the header, so generally it is
// only edited to add completely new USB interfaces or features.

// **************************************************************
//   USB Device
// **************************************************************

#define LSB(n) ((n) & 255)
#define MSB(n) (((n) >> 8) & 255)

#ifdef CDC_IAD_DESCRIPTOR
#ifndef DEVICE_CLASS
#define DEVICE_CLASS 0xEF
#endif
#ifndef DEVICE_SUBCLASS
#define DEVICE_SUBCLASS 0x02
#endif
#ifndef DEVICE_PROTOCOL
#define DEVICE_PROTOCOL 0x01
#endif
#endif

// USB Device Descriptor.  The USB host reads this first, to learn
// what type of device is connected.
static uint8_t device_descriptor[] = {
    18,         // bLength
    1,          // bDescriptorType
    0x00, 0x02, // bcdUSB
#ifdef DEVICE_CLASS
    DEVICE_CLASS, // bDeviceClass
#else
    0,
#endif
#ifdef DEVICE_SUBCLASS
    DEVICE_SUBCLASS, // bDeviceSubClass
#else
    0,
#endif
#ifdef DEVICE_PROTOCOL
    DEVICE_PROTOCOL, // bDeviceProtocol
#else
    0,
#endif
    EP0_SIZE,                         // bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),   // idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID), // idProduct
#ifdef BCD_DEVICE
    LSB(BCD_DEVICE), MSB(BCD_DEVICE), // bcdDevice
#else
// For USB types that don't explicitly define BCD_DEVICE,
// use the minor version number to help teensy_ports
// identify which Teensy model is used.
#if defined(__IMXRT1062__) && defined(ARDUINO_TEENSY40)
    0x79, 0x02, // Teensy 4.0
#elif defined(__IMXRT1062__) && defined(ARDUINO_TEENSY41)
    0x80, 0x02, // Teensy 4.1
#elif defined(__IMXRT1062__) && defined(ARDUINO_TEENSY_MICROMOD)
    0x81, 0x02, // Teensy MicroMod
#else
    0x00, 0x02,
#endif
#endif
    1, // iManufacturer
    2, // iProduct
    3, // iSerialNumber
    1  // bNumConfigurations
};

PROGMEM static const uint8_t qualifier_descriptor[] = {
    // 9.6.2 Device_Qualifier, page 264
    10,         // bLength
    6,          // bDescriptorType
    0x00, 0x02, // bcdUSB
#ifdef DEVICE_CLASS
    DEVICE_CLASS, // bDeviceClass
#else
    0,
#endif
#ifdef DEVICE_SUBCLASS
    DEVICE_SUBCLASS, // bDeviceSubClass
#else
    0,
#endif
#ifdef DEVICE_PROTOCOL
    DEVICE_PROTOCOL, // bDeviceProtocol
#else
    0,
#endif
    EP0_SIZE, // bMaxPacketSize0
    1,        // bNumConfigurations
    0         // bReserved
};

#ifdef EXPERIMENTAL_INTERFACE
static uint8_t microsoft_os_string_desc[] = {
    18, 3,
    'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
    0xF8, 0 // GET_MS_DESCRIPTOR will use bRequest=0xF8
};
PROGMEM const uint8_t microsoft_os_compatible_id_desc[] = {
    40, 0, 0, 0,            // total length, 16 header + 24 function * 1
    0, 1, 4, 0,             // version 1.00, wIndex=4 (Compat ID)
    1, 0, 0, 0, 0, 0, 0, 0, // 1 function
    0, 1,
    'W', 'I', 'N', 'U', 'S', 'B', 0, 0, // compatibleID
    0, 0, 0, 0, 0, 0, 0, 0,             // subCompatibleID
    0, 0, 0, 0, 0, 0};
#if defined(MS_OS_EXT_PROP0_NAME) && defined(MS_OS_EXT_PROP0_DATA)
PROGMEM const uint8_t microsoft_os_properties_desc[] = {
    /* Header */
    142, 0, 0, 0, // dwLength (total length)
    0x00, 0x01,   // bcdVersion 1.00
    0x05, 0x00,   // wIndex = 5 (Properties)
    0x01, 0x00,   // wCount = 1

    /* Property Section 0 */
    132, 0, 0, 0,                      // dwSize (size of this section)
    MS_OS_EXT_PROP0_DATA_TYP, 0, 0, 0, // dwPropertyDataType (1 = Unicode string)
    40, 0,                             // wPropertyNameLength (bytes, UTF-16LE, includes null)
    /* PropertyName: "DeviceInterfaceGUID" UTF-16LE with terminating NULL */
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0, 'G', 0, 'U', 0, 'I', 0, 'D', 0, 0, 0,
    78, 0, 0, 0, // dwPropertyDataLength (bytes, UTF-16LE, includes null)
    /* PropertyData: "{CDB3B5AD-293B-4663-AA36-1AAE46463776}" UTF-16LE + NULL */
    '{', 0, 'C', 0, 'D', 0, 'B', 0, '3', 0, 'B', 0, '5', 0, 'A', 0, 'D', 0, '-', 0, '2', 0, '9', 0, '3', 0, 'B', 0, '-', 0, '4', 0, '6', 0, '6', 0, '3', 0, '-', 0, 'A', 0, 'A', 0, '3', 0, '6', 0, '-', 0, '1', 0, 'A', 0, 'A', 0, 'E', 0, '4', 0, '6', 0, '4', 0, '6', 0, '3', 0, '7', 0, '7', 0, '6', 0, '}', 0, 0, 0};
#endif
#endif

__attribute__((section(".dmabuffers"), aligned(32)))
uint8_t usb_compatible_id_desc_buffer[64];

__attribute__((section(".dmabuffers"), aligned(32)))
uint8_t usb_properties_desc_buffer[160];

// **************************************************************
//   USB Descriptor Sizes
// **************************************************************

// pre-compute the size and position of everything in the config descriptor
//
#define CONFIG_HEADER_DESCRIPTOR_SIZE 9

#define CMSIS_DAP_INTERFACE_DESC_POS CONFIG_HEADER_DESCRIPTOR_SIZE
#ifdef CMSIS_DAP_IF0_NUM
#define CMSIS_DAP_INTERFACE_DESC_SIZE 9 + 7 + 7 // interface + 2 endpoints
#else
#define CMSIS_DAP_INTERFACE_DESC_SIZE 0
#endif

#define EXPERIMENTAL_INTERFACE_DESC_POS CMSIS_DAP_INTERFACE_DESC_POS + CMSIS_DAP_INTERFACE_DESC_SIZE
#ifdef EXPERIMENTAL_INTERFACE
// #define EXPERIMENTAL_INTERFACE_DESC_SIZE 9 + 7 + 7
#define EXPERIMENTAL_INTERFACE_DESC_SIZE 0
#else
#define EXPERIMENTAL_INTERFACE_DESC_SIZE 0
#endif

#define CONFIG_DESC_SIZE EXPERIMENTAL_INTERFACE_DESC_POS + EXPERIMENTAL_INTERFACE_DESC_SIZE

// **************************************************************
//   USB Configuration
// **************************************************************

// USB Configuration Descriptor.  This huge descriptor tells all
// of the devices capabilities.

PROGMEM const uint8_t usb_config_descriptor_480[CONFIG_DESC_SIZE] = {
    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9,                     // bLength;
    2,                     // bDescriptorType;
    LSB(CONFIG_DESC_SIZE), // wTotalLength
    MSB(CONFIG_DESC_SIZE),
    NUM_INTERFACE, // bNumInterfaces
    1,             // bConfigurationValue
    0,             // iConfiguration
    0xC0,          // bmAttributes
    50,            // bMaxPower

#ifdef CMSIS_DAP_IF0_NUM
    // CMSIS-DAP Interface 0 (480 Mbit/sec speed)
    // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,                           // bLength
    4,                           // bDescriptorType
    CMSIS_DAP_IF0_NUM,           // bInterfaceNumber
    0,                           // bAlternateSetting
    CMSIS_DAP_IF0_NUM_ENDPOINTS, // bNumEndpoints
    CMSIS_DAP_IF0_CLASS,         // bInterfaceClass
    CMSIS_DAP_IF0_SUBCLASS,      // bInterfaceSubClass
    CMSIS_DAP_IF0_PROTOCOL,      // bInterfaceProtocol
    0,                           // iInterface
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                                              // bLength
    5,                                                              // bDescriptorType
    CMSIS_DAP_IF0_EP2_ADDR,                                         // bEndpointAddress
    0x02,                                                           // bmAttributes (0x02=bulk)
    LSB(CMSIS_DAP_IF0_EP2_HS_WMAX), MSB(CMSIS_DAP_IF0_EP2_HS_WMAX), // wMaxPacketSize
    CMSIS_DAP_IF0_EP2_BINTERVAL,                                    // bInterval
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                                              // bLength
    5,                                                              // bDescriptorType
    CMSIS_DAP_IF0_EP3_ADDR,                                         // bEndpointAddress
    0x02,                                                           // bmAttributes (0x02=bulk)
    LSB(CMSIS_DAP_IF0_EP3_HS_WMAX), MSB(CMSIS_DAP_IF0_EP3_HS_WMAX), // wMaxPacketSize
    CMSIS_DAP_IF0_EP3_BINTERVAL,                                    // bInterval
#endif                                                              // CMSIS_DAP_IF0_NUM
};

PROGMEM const uint8_t usb_config_descriptor_12[CONFIG_DESC_SIZE] = {
    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9,                     // bLength;
    2,                     // bDescriptorType;
    LSB(CONFIG_DESC_SIZE), // wTotalLength
    MSB(CONFIG_DESC_SIZE),
    NUM_INTERFACE, // bNumInterfaces
    1,             // bConfigurationValue
    0,             // iConfiguration
    0xC0,          // bmAttributes
    50,            // bMaxPower

#ifdef CMSIS_DAP_IF0_NUM
    // CMSIS-DAP Interface 0 (12 Mbit/sec speed)
    // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,                           // bLength
    4,                           // bDescriptorType
    CMSIS_DAP_IF0_NUM,           // bInterfaceNumber
    0,                           // bAlternateSetting
    CMSIS_DAP_IF0_NUM_ENDPOINTS, // bNumEndpoints
    CMSIS_DAP_IF0_CLASS,         // bInterfaceClass
    CMSIS_DAP_IF0_SUBCLASS,      // bInterfaceSubClass
    CMSIS_DAP_IF0_PROTOCOL,      // bInterfaceProtocol
    0,                           // iInterface
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                                              // bLength
    5,                                                              // bDescriptorType
    CMSIS_DAP_IF0_EP2_ADDR,                                         // bEndpointAddress
    0x02,                                                           // bmAttributes (0x02=bulk)
    LSB(CMSIS_DAP_IF0_EP2_FS_WMAX), MSB(CMSIS_DAP_IF0_EP2_FS_WMAX), // wMaxPacketSize
    CMSIS_DAP_IF0_EP2_BINTERVAL,                                    // bInterval
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                                              // bLength
    5,                                                              // bDescriptorType
    CMSIS_DAP_IF0_EP3_ADDR,                                         // bEndpointAddress
    0x02,                                                           // bmAttributes (0x02=bulk)
    LSB(CMSIS_DAP_IF0_EP3_FS_WMAX), MSB(CMSIS_DAP_IF0_EP3_FS_WMAX), // wMaxPacketSize
    CMSIS_DAP_IF0_EP3_BINTERVAL,                                    // bInterval
#endif                                                              // CMSIS_DAP_IF0_NUM
};

__attribute__((section(".dmabuffers"), aligned(32)))
uint8_t usb_descriptor_buffer[CONFIG_DESC_SIZE];

// **************************************************************
//   String Descriptors
// **************************************************************

// The descriptors above can provide human readable strings,
// referenced by index numbers.  These descriptors are the
// actual string data

/* defined in usb_names.h
struct usb_string_descriptor_struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wString[];
};
*/

extern struct usb_string_descriptor_struct usb_string_manufacturer_name
    __attribute__((weak, alias("usb_string_manufacturer_name_default")));
extern struct usb_string_descriptor_struct usb_string_product_name
    __attribute__((weak, alias("usb_string_product_name_default")));
extern struct usb_string_descriptor_struct usb_string_serial_number
    __attribute__((weak, alias("usb_string_serial_number_default")));

PROGMEM const struct usb_string_descriptor_struct string0 = {
    4,
    3,
    {0x0409}};

PROGMEM const struct usb_string_descriptor_struct usb_string_manufacturer_name_default = {
    2 + MANUFACTURER_NAME_LEN * 2,
    3,
    MANUFACTURER_NAME};
PROGMEM const struct usb_string_descriptor_struct usb_string_product_name_default = {
    2 + PRODUCT_NAME_LEN * 2,
    3,
    PRODUCT_NAME};
struct usb_string_descriptor_struct usb_string_serial_number_default = {
    12,
    3,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

void usb_init_serialnumber(void)
{
    char buf[11];
    uint32_t i, num;

    num = HW_OCOTP_MAC0 & 0xFFFFFF;
    // add extra zero to work around OS-X CDC-ACM driver bug
    if (num < 10000000)
        num = num * 10;
    ultoa(num, buf, 10);
    for (i = 0; i < 10; i++)
    {
        char c = buf[i];
        if (!c)
            break;
        usb_string_serial_number_default.wString[i] = c;
    }
    usb_string_serial_number_default.bLength = i * 2 + 2;
}

// **************************************************************
//   Descriptors List
// **************************************************************

// This table provides access to all the descriptor data above.

const usb_descriptor_list_t usb_descriptor_list[] = {
    // wValue, wIndex, address,          length
    {0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
    {0x0600, 0x0000, qualifier_descriptor, sizeof(qualifier_descriptor)},
    {0x0200, 0x0000, usb_config_descriptor_480, CONFIG_DESC_SIZE},
    {0x0700, 0x0000, usb_config_descriptor_12, CONFIG_DESC_SIZE},
#ifdef EXPERIMENTAL_INTERFACE
    {0x03EE, 0x0000, microsoft_os_string_desc, 18},
    {0x0000, 0xEE04, microsoft_os_compatible_id_desc, 40},
#if defined(MS_OS_EXT_PROP0_NAME) && defined(MS_OS_EXT_PROP0_DATA)
    {0x0000, 0xEE05, microsoft_os_properties_desc, 142},
#endif
#endif
    {0x0300, 0x0000, (const uint8_t *)&string0, 0},
    {0x0301, 0x0409, (const uint8_t *)&usb_string_manufacturer_name, 0},
    {0x0302, 0x0409, (const uint8_t *)&usb_string_product_name, 0},
    {0x0303, 0x0409, (const uint8_t *)&usb_string_serial_number, 0},
    {0, 0, NULL, 0}};

#endif // NUM_ENDPOINTS
// #endif // F_CPU >= 20 MHz
