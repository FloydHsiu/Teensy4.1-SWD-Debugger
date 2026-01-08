#include "cmsis_dap/task_cmsis_dap.h"
#include "DAP_config.h"
#include "cmsis_dap/usb_cmsis_dap.h"

#include <DAP.h>
#include <FreeRTOS.h>
#include <pgmspace.h>
#include <queue.h>
#include <semphr.h>
#include <string.h>
#include <task.h>
#include <timers.h>

// Clock Macros
#define MAX_SWJ_CLOCK(delay_cycles) \
    ((CPU_CLOCK / 2U) / (IO_PORT_WRITE_CYCLES + delay_cycles))

#undef DELAY_SLOW_CYCLES
#define DELAY_SLOW_CYCLES 1U

DMAMEM static uint8_t USB_Request[DAP_PACKET_SIZE];  // Request  Buffer
DMAMEM static uint8_t USB_Response[DAP_PACKET_SIZE]; // Response Buffer

// Common clock delay calculation routine
//   clock:    requested SWJ frequency in Hertz
static void Set_Clock_Delay(uint32_t clock)
{
    uint32_t delay;

    if (clock >= MAX_SWJ_CLOCK(DELAY_FAST_CYCLES))
    {
        DAP_Data.fast_clock = 1U;
        DAP_Data.clock_delay = 1U;
    }
    else
    {
        DAP_Data.fast_clock = 0U;

        delay = ((CPU_CLOCK / 2U) + (clock - 1U)) / clock;
        if (delay > IO_PORT_WRITE_CYCLES)
        {
            delay -= IO_PORT_WRITE_CYCLES;
            delay = (delay + (DELAY_SLOW_CYCLES - 1U)) / DELAY_SLOW_CYCLES;
            // delay fix
            delay = delay - 16;
        }
        else
        {
            delay = 1U;
        }

        DAP_Data.clock_delay = delay;
    }
}

// Process SWJ Clock command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
static uint32_t CmsisDapSwjClock(const uint8_t *request, uint8_t *response)
{
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
    uint32_t clock;

    clock = (uint32_t)(*(request + 0) << 0) |
            (uint32_t)(*(request + 1) << 8) |
            (uint32_t)(*(request + 2) << 16) |
            (uint32_t)(*(request + 3) << 24);

    if (clock == 0U)
    {
        *response = DAP_ERROR;
        return ((4U << 16) | 1U);
    }

    Set_Clock_Delay(clock);

    *response = DAP_OK;
#else
    *response = DAP_ERROR;
#endif

    return ((4U << 16) | 1U);
}

// Process DAP command request and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t CmsisDapProcessCommand(const uint8_t *request, uint8_t *response)
{
    uint32_t num;
    uint8_t specified_process = 0U;

    switch (*request)
    {
        case ID_DAP_SWJ_Clock:
            specified_process = 1U;
            break;
        default:
            break;
    }

    if (specified_process)
    {
        *response++ = *request;

        switch (*request++)
        {
            case ID_DAP_SWJ_Clock:
                num = CmsisDapSwjClock(request, response);
                break;
        }

        return ((1U << 16) + 1U + num);
    }
    else
    {
        return DAP_ProcessCommand(request, response);
    }
}

// Execute DAP command (process request and prepare response)
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t CmsisDapExecuteCommand(const uint8_t *request, uint8_t *response)
{
    uint32_t cnt, num, n;

    if (*request == ID_DAP_ExecuteCommands)
    {
        *response++ = *request++;
        cnt = *request++;
        *response++ = (uint8_t)cnt;
        num = (2U << 16) | 2U;
        while (cnt--)
        {
            n = CmsisDapProcessCommand(request, response);
            num += n;
            request += (uint16_t)(n >> 16);
            response += (uint16_t)n;
        }
        return (num);
    }

    return CmsisDapProcessCommand(request, response);
}

/*-----------------------------------------------------------*/

void CmsisDapTask(void *parameters)
{
    (void)printf("CmsisDapTask\n");
    DAP_Setup();
    uint16_t request_len = 0, response_len = 0;
    for (;;)
    {
        request_len = usb_cmsis_dap_recv(USB_Request, 100);
        response_len = 0;
        if (request_len > 0)
        {
            memset(USB_Response, 0, DAP_PACKET_SIZE);
            portDISABLE_INTERRUPTS();
            response_len = CmsisDapExecuteCommand(USB_Request, USB_Response);
            portENABLE_INTERRUPTS();
            if (usb_cmsis_dap_send(USB_Response, response_len, 100) == 0)
            {
                printf("usb_cmsis_dap_send failed\n");
            }
        }

        if (!usb_cmsis_dap_rx_available())
        {
            taskYIELD();
        }
    }
}