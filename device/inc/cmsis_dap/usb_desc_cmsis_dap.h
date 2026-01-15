#pragma once

#include <stddef.h>
#include <stdint.h>

#define ENDPOINT_TRANSMIT_UNUSED      0x00020000
#define ENDPOINT_TRANSMIT_ISOCHRONOUS 0x00C40000
#define ENDPOINT_TRANSMIT_BULK        0x00C80000
#define ENDPOINT_TRANSMIT_INTERRUPT   0x00CC0000
#define ENDPOINT_RECEIVE_UNUSED       0x00000002
#define ENDPOINT_RECEIVE_ISOCHRONOUS  0x000000C4
#define ENDPOINT_RECEIVE_BULK         0x000000C8
#define ENDPOINT_RECEIVE_INTERRUPT    0x000000CC

#if defined(USB_CMSIS_DAP)
#define VENDOR_ID  0x16C0
#define PRODUCT_ID 0x0483
#define MANUFACTURER_NAME                                     \
    {                                                         \
        'T', 'e', 'e', 'n', 's', 'y', 'd', 'u', 'i', 'n', 'o' \
    }
#define MANUFACTURER_NAME_LEN 11
#define PRODUCT_NAME                                                                                            \
    {                                                                                                           \
        'T', 'e', 'e', 'n', 's', 'y', 'd', 'u', 'i', 'n', 'o', ' ', 'C', 'M', 'S', 'I', 'S', '-', 'D', 'A', 'P' \
    }
#define PRODUCT_NAME_LEN 21
#define PRODUCT_FIRMWARE_VERSION \
    {                            \
        'V', '0', '.', '1'       \
    }
#define PRODUCT_FIRMWARE_VERSION_LEN 4

#define EP0_SIZE              64
#define NUM_ENDPOINTS         3
#define NUM_INTERFACE         1
#define DEVICE_CLASS          0xFF
#define DEVICE_SUBCLASS       0x00
#define DEVICE_PROTOCOL       0x00
#define CMSIS_DAP_RX_ENDPOINT 2
#define CMSIS_DAP_TX_ENDPOINT 3

// CMSIS-DAP CustomClass Interface 0 Configuration
#define CMSIS_DAP_INTERFACE         0
#define CMSIS_DAP_IF0_NUM           0
#define CMSIS_DAP_IF0_CLASS         0xFF
#define CMSIS_DAP_IF0_SUBCLASS      0x00
#define CMSIS_DAP_IF0_PROTOCOL      0x00
#define CMSIS_DAP_IF0_NUM_ENDPOINTS 2

// IF0 EP1: OUT, address 0x02 (bulk, FS:64/HS:512) RX
#define CMSIS_DAP_IF0_EP2_ADDR      0x02
#define CMSIS_DAP_IF0_EP2_FS_WMAX   64
#define CMSIS_DAP_IF0_EP2_HS_WMAX   512
#define CMSIS_DAP_IF0_EP2_BINTERVAL 0

// IF0 EP2: IN, address 0x83 (bulk, FS:64/HS:512) TX
#define CMSIS_DAP_IF0_EP3_ADDR      0x83
#define CMSIS_DAP_IF0_EP3_FS_WMAX   64
#define CMSIS_DAP_IF0_EP3_HS_WMAX   512
#define CMSIS_DAP_IF0_EP3_BINTERVAL 0

#define EXPERIMENTAL_INTERFACE 1

#define ENDPOINT2_CONFIG         ENDPOINT_RECEIVE_BULK + ENDPOINT_TRANSMIT_UNUSED
#define ENDPOINT3_CONFIG         ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_BULK
#define MS_OS_EXT_COMPAT_ID_SIZE 40
#define MS_OS_EXT_COMPAT_ID      "WINUSB"
#define MS_OS_EXT_PROP0_SIZE     142
#define MS_OS_EXT_PROP0_DATA_TYP 1
#define MS_OS_EXT_PROP0_NAME     L"DeviceInterfaceGUID"
#define MS_OS_EXT_PROP0_DATA     L"{CDB3B5AD-293B-4663-AA36-1AAE46463776}"
#endif

#ifdef USB_DESC_LIST_DEFINE
#if defined(NUM_ENDPOINTS) && NUM_ENDPOINTS > 0
// NUM_ENDPOINTS = number of non-zero endpoints (0 to 7)
extern const uint32_t usb_endpoint_config_table[NUM_ENDPOINTS];

typedef struct
{
    uint16_t wValue;
    uint16_t wIndex;
    const uint8_t *addr;
    uint16_t length;
} usb_descriptor_list_t;

extern const usb_descriptor_list_t usb_descriptor_list[];
#endif // NUM_ENDPOINTS
#endif // USB_DESC_LIST_DEFINE