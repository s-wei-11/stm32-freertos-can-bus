#include "step_28byj48.h"

/* 
 * 四相八拍时序查找表 (A-AB-B-BC-C-CD-D-DA-A)
 * 1 代表使能通电 (单片机输出高电平，ULN2003 对地导通线圈)
 * 0 代表断电
 */
static const uint8_t BEAT_TABLE[8][4] = {
    {1, 0, 0, 0}, // A
    {1, 1, 0, 0}, // AB
    {0, 1, 0, 0}, // B
    {0, 1, 1, 0}, // BC
    {0, 0, 1, 0}, // C
    {0, 0, 1, 1}, // CD
    {0, 0, 0, 1}, // D
    {1, 0, 0, 1}  // DA
};

/**
 * @brief 原子化设置引脚高低电平 (使用 BSRR 寄存器)
 */
static inline void Pin_Write(GPIO_TypeDef *port, uint16_t pin, uint8_t state)
{
    if (state) {
        port->BSRR = pin;                      // 置高电平
    } else {
        port->BSRR = (uint32_t)pin << 16u;     // 置低电平
    }
}

/**
 * @brief 初始化步进电机引脚配置
 * @note  引脚需在 CubeMX 中配置为 GPIO_MODE_OUTPUT_PP (推挽输出，接 ULN2003 输入端)
 */
bool Stepper_Init(Stepper_28BYJ48_t *dev, GPIO_TypeDef *ports[4], const uint16_t pins[4])
{
    if (dev == NULL || ports == NULL || pins == NULL) {
        return false;
    }

    for (uint8_t i = 0; i < 4; i++) {
        dev->port[i] = ports[i];
        dev->pin[i]  = pins[i];
    }

    dev->step_idx = 0;
    
    // 初始化完成后立刻切断线圈电流，防止上电静止时发烫
    Stepper_PowerOff(dev);
    return true;
}

/**
 * @brief 单步进阶函数 (核心控制逻辑)
 * @param dir 旋转方向：STEPPER_DIR_CW 或 STEPPER_DIR_CCW
 */
void Stepper_Step(Stepper_28BYJ48_t *dev, Stepper_Dir_t dir)
{
    if (dev == NULL) {
        return;
    }

    // 1. 根据方向更新 0~7 环形索引
    if (dir == STEPPER_DIR_CW) {
        dev->step_idx = (dev->step_idx + 1) & 0x07; // 相当于 % 8
    } else {
        dev->step_idx = (dev->step_idx - 1 + 8) & 0x07;
    }

    // 2. 根据查找表依次翻转 4 个相的引脚电平[cite: 1]
    for (uint8_t phase = 0; phase < 4; phase++) {
        Pin_Write(dev->port[phase], dev->pin[phase], BEAT_TABLE[dev->step_idx][phase]);
    }

    dev->is_idle = false;
}

/**
 * @brief 停机断电保护 (关键工业安全函数)
 * @note  将 4 个引脚全部置低，切断达林顿管电流，杜绝静止发热
 */
void Stepper_PowerOff(Stepper_28BYJ48_t *dev)
{
    if (dev == NULL) {
        return;
    }

    for (uint8_t phase = 0; phase < 4; phase++) {
        Pin_Write(dev->port[phase], dev->pin[phase], 0);
    }

    dev->is_idle = true;
}

/**
 * @brief 将目标角度转换为脉冲步数
 */
uint32_t Stepper_AngleToSteps(float angle_deg)
{
    if (angle_deg < 0.0f) {
        angle_deg = -angle_deg;
    }
    return (uint32_t)((angle_deg / 360.0f) * STEPPER_TOTAL_STEPS_PER_REV);
}