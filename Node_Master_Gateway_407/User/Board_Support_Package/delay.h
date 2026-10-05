#ifndef __delay_h
#define __delay_h

#include "main.h"

void dwt_init(void);

void delay_us(uint32_t us);
void delay_us_systick(uint32_t us);

#endif
