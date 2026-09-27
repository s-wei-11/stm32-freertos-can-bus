#ifndef STEP_28BYJ48_H
#define STEP_28BYJ48_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define STEPPER_TOTAL_STEPS_PER_REV  4096  // 8 拍模式下，减速后输出轴旋转 1 圈的脉冲节拍数 (64 * 64)

typedef enum {
    STEPPER_DIR_CW  =  1, // 顺时针正转
    STEPPER_DIR_CCW = -1  // 逆时针反转
} Stepper_Dir_t;

/**
 * @brief 28BYJ-48 步进电机句柄
 */
typedef struct {
    GPIO_TypeDef *port[4]; // IN1(A), IN2(B), IN3(C), IN4(D) 对应的 GPIO 端口
    uint16_t     pin[4];  // 对应的引脚号
    int8_t       step_idx;// 当前拍数索引 (0 ~ 7)
    bool         is_idle; // 是否处于空闲释放状态
} Stepper_28BYJ48_t;

/* ================= 外部 API ================= */
bool Stepper_Init(Stepper_28BYJ48_t *dev, GPIO_TypeDef *ports[4], const uint16_t pins[4]);
void Stepper_Step(Stepper_28BYJ48_t *dev, Stepper_Dir_t dir);
void Stepper_PowerOff(Stepper_28BYJ48_t *dev);
uint32_t Stepper_AngleToSteps(float angle_deg);

#endif /* STEPPER_28BYJ48_H */