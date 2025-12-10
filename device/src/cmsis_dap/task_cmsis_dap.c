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

DMAMEM static uint8_t USB_Request[DAP_PACKET_SIZE];  // Request  Buffer
DMAMEM static uint8_t USB_Response[DAP_PACKET_SIZE]; // Response Buffer

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
            response_len = DAP_ExecuteCommand(USB_Request, USB_Response);
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