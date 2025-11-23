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

DMAMEM static uint8_t USB_Request[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
DMAMEM static uint8_t USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE]; // Response Buffer

/*-----------------------------------------------------------*/

void CmsisDapTask(void *parameters)
{
    (void)printf("CmsisDapTask\n");
    uint16_t request_len = 0, response_len = 0;
    for (;;)
    {
        request_len = usb_cmsis_dap_recv(USB_Request[0], 100);
        response_len = 0;
        if (request_len > 0)
        {
            // for (int i = 0; i < (request_len + 3) / 4; i++)
            // {
            //     printf("0x%x ", *(((uint32_t *)USB_Request[0]) + i));
            //     if (i % 4 == 3)
            //         printf("\n");
            // }
            // printf("\n");
            memset(USB_Response[0], 0, DAP_PACKET_SIZE);
            portDISABLE_INTERRUPTS();
            response_len = DAP_ExecuteCommand(USB_Request[0], USB_Response[0]);
            // printf("response_len: %d\n", response_len);
            // for (int i = 0; i < (response_len + 3) / 4; i++)
            // {
            //     printf("0x%x ", *(((uint32_t *)USB_Response[0]) + i));
            //     if (i % 4 == 3)
            //         printf("\n");
            // }
            // printf("\n");
            portENABLE_INTERRUPTS();
            if (usb_cmsis_dap_send(USB_Response[0], response_len, 100) == 0)
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