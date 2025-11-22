/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
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

#include "cmsis_dap/usb_cmsis_dap.h"
#include "DAP_config.h"
#include "avr/pgmspace.h" // for PROGMEM, DMAMEM, FASTRUN
#include "cmsis_dap/usb_desc_cmsis_dap.h"
#include "core_pins.h" // for delay()
#include "usb_dev.h"
#include <string.h> // for memcpy()

#include "core_pins.h"
#include "debug/printf.h"

extern volatile uint8_t usb_high_speed;

#if defined(CMSIS_DAP_INTERFACE)
// At very slow CPU speeds, the OCRAM just isn't fast enough for
// USB to work reliably.  But the precious/limited DTCM is.  So
// as an ugly workaround, undefine DMAMEM so all buffers which
// would normally be allocated in OCRAM are placed in DTCM.
#if defined(F_CPU) && F_CPU < 30000000
#undef DMAMEM
#define DMAMEM
#endif

extern volatile uint8_t usb_configuration;
extern volatile uint8_t usb_high_speed;

#define TX_NUM  DAP_PACKET_COUNT
#define TX_SIZE DAP_PACKET_SIZE
static transfer_t tx_transfer[TX_NUM] __attribute__((used, aligned(32)));
DMAMEM static uint8_t tx_buffer[TX_SIZE * TX_NUM] __attribute__((aligned(32)));
static uint8_t tx_head = 0;
static uint16_t tx_packet_size = 0;

#define RX_NUM  DAP_PACKET_COUNT
#define RX_SIZE DAP_PACKET_SIZE
static transfer_t rx_transfer[RX_NUM] __attribute__((used, aligned(32)));
DMAMEM static uint8_t rx_buffer[RX_NUM * RX_SIZE] __attribute__((aligned(32)));
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static uint16_t rx_packet_size = 0;
static uint8_t rx_list[RX_NUM + 1];
static int rx_list_transfer_len[RX_NUM + 1];

static void rx_queue_transfer(int i);
static void rx_event(transfer_t *t);

void usb_cmsis_dap_configure(void)
{
    int i;

    printf("%s\n", __func__);
    if (usb_high_speed)
    {
        tx_packet_size = CMSIS_DAP_IF0_EP3_HS_WMAX;
        rx_packet_size = CMSIS_DAP_IF0_EP2_HS_WMAX;
    }
    else
    {
        tx_packet_size = CMSIS_DAP_IF0_EP3_FS_WMAX;
        rx_packet_size = CMSIS_DAP_IF0_EP2_FS_WMAX;
    }
    memset(tx_transfer, 0, sizeof(tx_transfer));
    memset(rx_transfer, 0, sizeof(rx_transfer));
    tx_head = 0;
    rx_head = 0;
    rx_tail = 0;
    usb_config_tx(CMSIS_DAP_TX_ENDPOINT, tx_packet_size, 0, NULL);
    usb_config_rx(CMSIS_DAP_RX_ENDPOINT, rx_packet_size, 0, rx_event);
    for (i = 0; i < RX_NUM; i++)
        rx_queue_transfer(i);
}

/*************************************************************************/
/**                               Receive                               **/
/*************************************************************************/

static void rx_queue_transfer(int i)
{
    void *buffer = rx_buffer + i * RX_SIZE;
    NVIC_DISABLE_IRQ(IRQ_USB1);
    usb_prepare_transfer(rx_transfer + i, buffer, rx_packet_size, i);
    arm_dcache_delete(buffer, rx_packet_size);
    usb_receive(CMSIS_DAP_RX_ENDPOINT, rx_transfer + i);
    NVIC_ENABLE_IRQ(IRQ_USB1);
}

static void rx_event(transfer_t *t)
{
    int i = t->callback_param;
    printf("rx event i=%d\n", i);
    //  received a packet with data
    uint32_t head = rx_head;
    if (++head > RX_NUM)
        head = 0;
    rx_list[head] = i;
    // remember how many bytes were actually sent by host...
    int len = rx_packet_size - ((t->status >> 16) & 0x7FFF);
    rx_list_transfer_len[head] = len;
    rx_head = head;
}

int usb_cmsis_dap_recv(void *buffer, uint32_t timeout)
{
    uint32_t wait_begin_at = systick_millis_count;
    uint32_t tail = rx_tail;
    while (1)
    {
        if (!usb_configuration)
            return -1; // usb not enumerated by host
        if (tail != rx_head)
            break;
        if (systick_millis_count - wait_begin_at >= timeout)
        {
            return 0;
        }
    }
    printf("usb_cmsis_dap_recv\n");
    if (++tail > RX_NUM)
        tail = 0;
    uint32_t i = rx_list[tail];
    int len = rx_list_transfer_len[tail];
    rx_tail = tail;

    uint8_t *rx_item_buffer = rx_buffer + i * RX_SIZE;
    // BUGBUG Should we use the
    memcpy(buffer, rx_item_buffer, len);
    rx_queue_transfer(i);
    // memset(rx_transfer, 0, sizeof(rx_transfer));
    // usb_prepare_transfer(rx_transfer + 0, rx_buffer, rx_packet_size, 0);
    // usb_receive(MTP_RX_ENDPOINT, rx_transfer + 0);
    return len;
}

/*************************************************************************/
/**                             Send                                    **/
/*************************************************************************/
int usb_cmsis_dap_send(const void *buffer, uint32_t len, uint32_t timeout)
{
    transfer_t *xfer = tx_transfer + tx_head;
    uint32_t wait_begin_at = systick_millis_count;

    while (1)
    {
        if (!usb_configuration)
            return -1; // usb not enumerated by host
        uint32_t status = usb_transfer_status(xfer);
        if (!(status & 0x80))
            break; // transfer descriptor ready
        if (systick_millis_count - wait_begin_at > timeout)
            return 0;
    }
    uint8_t *txdata = tx_buffer + (tx_head * TX_SIZE);
    memcpy(txdata, buffer, len);
    arm_dcache_flush_delete(txdata, tx_packet_size);
    usb_prepare_transfer(xfer, txdata, len, 0);
    usb_transmit(CMSIS_DAP_TX_ENDPOINT, xfer);
    if (++tx_head >= TX_NUM)
        tx_head = 0;
    return len;
}

#endif