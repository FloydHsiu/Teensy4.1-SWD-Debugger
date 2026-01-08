#include "DAP_config.h"
#include "cmsis_dap/task_cmsis_dap.h"
#include "cmsis_dap/usb_cmsis_dap.h"

#include <DAP.h>
#include <FreeRTOS.h>
#include <pgmspace.h>
#include <queue.h>
#include <semphr.h>
#include <string.h>
#include <task.h>
#include <timers.h>

extern DAP_Data_t DAP_Data;

#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)

#define SWD_CLK()        \
    PIN_SWCLK_TCK_CLR(); \
    PIN_DELAY();         \
    PIN_SWCLK_TCK_SET(); \
    PIN_DELAY()

#define SWD_WRITE_BIT(bit) \
    PIN_SWDIO_OUT(bit);    \
    PIN_SWCLK_TCK_CLR();   \
    PIN_DELAY();           \
    PIN_SWCLK_TCK_SET();   \
    PIN_DELAY()

#define SWD_READ_BIT(bit) \
    PIN_SWCLK_TCK_CLR();  \
    PIN_DELAY();          \
    bit = PIN_SWDIO_IN(); \
    PIN_SWCLK_TCK_SET();  \
    PIN_DELAY()

// ARM CoreSight SWD-DP packet request values
#define SW_IDCODE_RD   0xA5
#define SW_ABORT_WR    0x81
#define SW_CTRLSTAT_RD 0x8D
#define SW_CTRLSTAT_WR 0xA9
#define SW_RESEND_RD   0x95
#define SW_SELECT_WR   0xB1
#define SW_RDBUFF_RD   0xBD

// ARM CoreSight SW-DP packet request masks
#define SW_REQ_PARK_START 0x81
#define SW_REQ_PARITY     0x20
#define SW_REQ_A32        0x18
#define SW_REQ_RnW        0x04
#define SW_REQ_APnDP      0x02

// ARM CoreSight SW-DP packet acknowledge values
#define SW_ACK_OK         0x1
#define SW_ACK_WAIT       0x2
#define SW_ACK_FAULT      0x4
#define SW_ACK_PARITY_ERR 0x8

__STATIC_FORCEINLINE void swd_write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        PIN_SWDIO_OUT(data & 0x01);
        data >>= 1;
        SWD_CLK();
    }
}

__STATIC_FORCEINLINE uint8_t swd_read_byte()
{
    uint8_t data = 0;
    uint8_t bit = 0;
    for (int i = 0; i < 8; i++)
    {
        SWD_READ_BIT(bit);
        data <<= 1;
        data |= bit;
    }

    return data;
}

__STATIC_FORCEINLINE uint32_t swd_packet(uint8_t request)
{
    uint8_t read_bit = 0;
    uint8_t io_byte = 0;
    uint8_t ack = 0;
    uint32_t id_code = 0;

    swd_write_byte(request);

    PIN_SWDIO_OUT_DISABLE();
    SWD_CLK();

    SWD_READ_BIT(read_bit);
    io_byte |= read_bit << 0;

    SWD_READ_BIT(read_bit);
    io_byte |= read_bit << 1;

    SWD_READ_BIT(read_bit);
    io_byte |= read_bit << 2;

    ack = io_byte;

    if (ack == SW_ACK_OK)
    {
        if (request & SW_REQ_RnW)
        {
            io_byte = swd_read_byte();
            id_code |= io_byte << 0;
            io_byte = swd_read_byte();
            id_code |= io_byte << 8;
            io_byte = swd_read_byte();
            id_code |= io_byte << 16;
            io_byte = swd_read_byte();
            id_code |= io_byte << 24;

            SWD_READ_BIT(read_bit);
        }
        else
        {
        }
    }

    PIN_SWDIO_OUT_ENABLE();
    PIN_SWDIO_OUT(1);
    SWD_CLK();

    printf("ack = 0x%02X\n", ack);
    return id_code;
}

void test_dap_connect()
{
    uint32_t id_code = 0;
    // INIT
    DAP_SETUP();
    PORT_SWD_SETUP();

    // Reset
    for (int i = 0; i < 64; i++)
    {
        SWD_CLK();
    }

    swd_write_byte(0x9E);
    swd_write_byte(0xE7);

    // Reset
    for (int i = 0; i < 64; i++)
    {
        SWD_CLK();
    }

    swd_write_byte(0x00);

    id_code = swd_packet(SW_IDCODE_RD);
    // id_code = swd_packet(0x02);

    swd_write_byte(0x00);

    printf("id_code = 0x%08X\n", id_code);
}