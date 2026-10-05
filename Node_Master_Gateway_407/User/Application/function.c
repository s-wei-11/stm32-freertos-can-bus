#include "function.h"
#include "portmacro.h"
#include "projdefs.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/cdefs.h>

SemaphoreHandle_t uart_mux=NULL;    //建立句柄变量

void uartmux_init()
{
    if (uart_mux==NULL) {
        uart_mux = xSemaphoreCreateMutex(); //建锁
    }
}

void s_printf(const char * format,...)
{
    if(xSemaphoreTake(uart_mux, portMAX_DELAY)==pdTRUE) //一直等待取锁
    {
        va_list arg;    //定义句柄
        va_start(arg, format);
        vprintf(format, arg);
        va_end(arg);
      //  fflush(stdout);
        xSemaphoreGive(uart_mux);       //还锁
    }
    
}