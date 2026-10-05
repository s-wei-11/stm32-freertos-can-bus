#pragma once
#include "FreeRTOS.h"
#include "semphr.h"


void uartmux_init();
void s_printf(const char * format,...);

