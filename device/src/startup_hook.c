#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "imxrt.h"

extern unsigned long _estack;
extern volatile uint32_t F_CPU_ACTUAL;
extern void (* volatile _VectorsRam[NVIC_NUM_INTERRUPTS+16])(void);

void xPortPendSVHandler( void );
void vPortSVCHandler( void );
void xPortSysTickHandler( void );

void startup_late_hook(void)
{
    unsigned int i;

    portDISABLE_INTERRUPTS();

    // for (i=0; i < NVIC_NUM_INTERRUPTS + 16; i++) _VectorsRam[i] = &my_unused_interrupt_vector;

    _VectorsRam[0] = (void (*)(void))(&_estack);
    _VectorsRam[11] = vPortSVCHandler;
	_VectorsRam[14] = xPortPendSVHandler;

    SYST_CSR = 0;
    SYST_RVR = 0;
    SYST_CVR = 0;

    _VectorsRam[15] = xPortSysTickHandler;

    for (i=0; i < NVIC_NUM_INTERRUPTS; i++) NVIC_SET_PRIORITY(i, 240);
    //NVIC_DISABLE_IRQ(IRQ_USB1);
}