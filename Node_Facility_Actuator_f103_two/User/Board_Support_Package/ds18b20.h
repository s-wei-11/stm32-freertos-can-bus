#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

// 1-Wire 核心命令码
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

// DS18B20 设备结构体
typedef struct {
    GPIO_TypeDef *port;         // GPIO 端口 (如 GPIOA, GPIOB)
    uint16_t      pin;          // GPIO 引脚 (如 GPIO_PIN_12)
    bool          is_online;    // 在线自检标志
    float         temp_c;       // 解析后的摄氏度温度值
} Ds18bxx_t;

/* 对外接口函数声明 */
bool DS18B20_Init(Ds18bxx_t *dev, GPIO_TypeDef *port, uint16_t pin);
bool DS18B20_StartConversion(Ds18bxx_t *dev);
bool DS18B20_ReadTemp(Ds18bxx_t *dev, float *temp_c);

#endif /* __DS18B20_H */


