#ifndef __GPIO_INTERRUPT__
#define __GPIO_INTERRUPT__

#include "avr/pgmspace.h"

#include <stdint.h>

FASTRUN void irq_gpio6789(void);
void attachInterrupt(uint8_t pin, void (*function)(void), int mode);
void detachInterrupt(uint8_t pin);

#endif