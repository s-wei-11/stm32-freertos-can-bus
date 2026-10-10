#pragma once
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include "ecode.h"
#include "lcdxx_driver.h"   //显示
#include "spi.h"
#include "ds1302.h"

extern ecode_dev ecode_one;
extern lcdxx_dev st7735_one;    //tft屏幕对象
extern ds1302_t ds1302_one;  //定义ds1302对象

void frtos_app_init();//初始化设备

void s_printf(const char * format,...);

