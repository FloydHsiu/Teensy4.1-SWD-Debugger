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

#include "Arduino.h"
#include "debug/printf.h"
#include "imxrt.h"

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

/*-----------------------------------------------------------*/

static void LedBlinkTask(void *parameters) __attribute__((noreturn));

/*-----------------------------------------------------------*/

static void LedBlinkTask(void *parameters)
{
    (void)printf("LedBlinkTask\n");

    for (;;)
    {
        digitalToggleFast(3);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" int main(void)
{
    static StaticTask_t led_blink_task_tcb;
    static StackType_t led_blink_task_stack[configMINIMAL_STACK_SIZE];

    (void)printf("Example FreeRTOS Project\n");

    (void)xTaskCreateStatic(LedBlinkTask, "LedBlink", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1U,
                            &(led_blink_task_stack[0]), &(led_blink_task_tcb));

    /* Start the scheduler. */
    vTaskStartScheduler();

    for (;;)
    {
        /* Should not reach here. */
        (void)printf("Should not reach here\n");
    }

    return 0;
}

/*-----------------------------------------------------------*/

#if (configCHECK_FOR_STACK_OVERFLOW > 0)

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Check pcTaskName for the name of the offending task,
     * or pxCurrentTCB if pcTaskName has itself been corrupted. */
    (void)xTask;
    (void)pcTaskName;
    printf("Task %s stack overflow", pcTaskGetName);
}

#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW > 0 ) */

/*-----------------------------------------------------------*/
