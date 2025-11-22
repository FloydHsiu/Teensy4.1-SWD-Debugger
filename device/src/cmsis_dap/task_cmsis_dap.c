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
static uint16_t USB_RespSize[DAP_PACKET_COUNT];                        // Response Size

/*-----------------------------------------------------------*/

void CmsisDapTask(void *parameters)
{
    (void)printf("CmsisDapTask\n");
    for (;;)
    {
        int response_size = 0;
        if (usb_cmsis_dap_recv(USB_Request[0], 100) > 0)
        {
            // for (int i = 0; i < (DAP_PACKET_SIZE + 3) / 4; i++)
            // {
            //     printf("0x%x ", *(((uint32_t *)USB_Request[0]) + i));
            //     if (i % 4 == 3)
            //         printf("\n");
            // }
            memset(USB_Response[0], 0, DAP_PACKET_SIZE);
            response_size = DAP_ExecuteCommand(USB_Request[0], USB_Response[0]);
            usb_cmsis_dap_send(USB_Response[0], response_size, 100);
        }

        taskYIELD();
    }
}