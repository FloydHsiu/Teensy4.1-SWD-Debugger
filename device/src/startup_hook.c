#include "Arduino.h"
#include "imxrt.h"

#include <FreeRTOS.h>

#include "gpio_interrupt.h"

extern unsigned long _estack;
extern volatile uint32_t F_CPU_ACTUAL;
extern void (*volatile _VectorsRam[NVIC_NUM_INTERRUPTS + 16])(void);
extern volatile uint8_t usb_cdc_line_rtsdtr;

FASTRUN void xPortPendSVHandler(void);
FASTRUN void vPortSVCHandler(void);
FASTRUN void xPortSysTickHandler(void);
FASTRUN void GPIO15Handler(void);

void startup_late_hook(void)
{
    // interrupts expect to be disabled before vTaskStartScheduler
    portDISABLE_INTERRUPTS();

    // prvPortStartFirstTask - FreeRTOS use this to locate the stack
    _VectorsRam[0] = (void (*)(void))(&_estack);
    _VectorsRam[11] = vPortSVCHandler;
    _VectorsRam[14] = xPortPendSVHandler;

    // Reset handler set systick to avoid timing issue of usb initialization
    // Here reset systick register and its handle function provided by FreeRTOS
    _VectorsRam[15] = xPortSysTickHandler;
    SYST_CSR = 0;
    SYST_RVR = 0;
    SYST_CVR = 0;

    // Setup GPIO Pin 15 interrupt
    // pinMode(15, INPUT);
    // attachInterrupt(15, GPIO15Handler, RISING);

    // Setup GPIO for LED
    pinMode(13, OUTPUT);
    digitalWriteFast(13, LOW);

    unsigned int i;
    for (i = 0; i < NVIC_NUM_INTERRUPTS; i++)
        NVIC_SET_PRIORITY(i, 224);
}

// replace yield in Arduino
void yield(void) {}

void GPIO15Handler(void)
{
    printf("GPIO15Handler: %x\n", GPIO1_ISR);
}