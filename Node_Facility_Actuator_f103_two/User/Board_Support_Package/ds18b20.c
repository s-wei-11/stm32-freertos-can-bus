#include "ds18b20.h"
#include "Delay.h" // 包含你的微秒延时头文件 (需提供 delay_us)

/* ================== 1. 引脚底层操作宏 (你的命名风格) ================== */
#define SDA_High(x) ((x->port)->BSRR = (x->pin))
#define SDA_Low(x)  ((x->port)->BSRR = (uint32_t)(x->pin) << 16u)
#define SDA_Read(x) (((x->port)->IDR & (x->pin)) ? 1 : 0)

/* ================== 2. 细粒度微秒级临界区保护 ================== */
// 仅在 15us 核心跳变沿关中断，避免 FreeRTOS 调度或外部中断破坏通信时序
static inline uint32_t DS18B20_Critical_Enter(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void DS18B20_Critical_Exit(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* ================== 3. 1-Wire 底层物理时序 ================== */

/**
 * @brief  总线复位与存在脉冲检测 (不关中断，时序容限充足)
 * @retval true: 从机响应存在; false: 从机离线
 */
static bool DS18B20_Reset(Ds18bxx_t *dev)
{
    bool presence = false;

    SDA_Low(dev);
    delay_us(480); // 主机拉低复位脉冲 (480~960us)

    SDA_High(dev); // 主机释放总线 (开漏高阻，由 4.7k 上拉回弹)
    delay_us(60);  // 等待从机响应窗口 (15~60us 后从机拉低)

    // 检测到低电平说明从机在线拉低了总线
    if (SDA_Read(dev) == 0) {
        presence = true;
    }

    delay_us(420); // 补足剩余时间，等待从机释放总线
    return presence;
}

/**
 * @brief 向总线写 1 个 Bit
 */
static void DS18B20_WriteBit(Ds18bxx_t *dev, uint8_t bit)
{
    if (bit) {
        // 写 1 时序：拉低 1~15us 内释放
        uint32_t pri = DS18B20_Critical_Enter();
        SDA_Low(dev);
        delay_us(2);
        SDA_High(dev); // 释放总线
        DS18B20_Critical_Exit(pri);
        delay_us(60);
    } else {
        // 写 0 时序：拉低持续 60~120us
        uint32_t pri = DS18B20_Critical_Enter();
        SDA_Low(dev);
        delay_us(60);
        SDA_High(dev); // 释放总线
        DS18B20_Critical_Exit(pri);
        delay_us(2);
    }
}

/**
 * @brief 从总线读 1 个 Bit (15us 内严格采样的临界区)
 */
static uint8_t DS18B20_ReadBit(Ds18bxx_t *dev)
{
    uint8_t bit = 0;

    uint32_t pri = DS18B20_Critical_Enter();
    SDA_Low(dev);
    delay_us(2);       // 读时隙起始信号 (>1us)
    SDA_High(dev);     // 释放总线
    delay_us(10);      // 在 15us 窗口期内完成采样
    bit = SDA_Read(dev);
    DS18B20_Critical_Exit(pri);

    delay_us(50);      // 补满 60us 时隙槽
    return bit;
}

static void DS18B20_WriteByte(Ds18bxx_t *dev, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        DS18B20_WriteBit(dev, (byte >> i) & 0x01); // LSB 低位优先
    }
}



static uint8_t DS18B20_ReadByte(Ds18bxx_t *dev)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (DS18B20_ReadBit(dev)) {
            byte |= (1U << i); // LSB 低位优先
        }
    }
    return byte;
}

/* ================== 4. 业务层对外 API ================== */

/**
 * @brief  初始化 DS18B20
 * @note   引脚必须在 CubeMX 中配置为 GPIO_MODE_OUTPUT_OD (开漏)
 */
bool DS18B20_Init(Ds18bxx_t *dev, GPIO_TypeDef *port, uint16_t pin)
{
    dev->port = port;
    dev->pin  = pin;

    SDA_High(dev); // 空闲状态下默认释放总线
    delay_us(1000);

    dev->is_online = DS18B20_Reset(dev);
    return dev->is_online;
}

/**
 * @brief  向总线广播发送温度转换指令（非阻塞）
 * @note   发送后硬件转换需要约 750ms，不要在此处死等
 */
bool DS18B20_StartConversion(Ds18bxx_t *dev)
{
    if (!DS18B20_Reset(dev)) {
        dev->is_online = false;
        return false;
    }
    dev->is_online = true;

    DS18B20_WriteByte(dev, DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(dev, DS18B20_CMD_CONVERT_T);

    return true;
}

/**
 * @brief  读取暂存器并解析温度
 * @param  temp_c 接收温度浮点数的指针
 * @retval true: 读取成功; false: 传感器无响应/离线
 */
bool DS18B20_ReadTemp(Ds18bxx_t *dev, float *temp_c)
{
    if (!DS18B20_Reset(dev)) {
        dev->is_online = false;
        return false;
    }

    DS18B20_WriteByte(dev, DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(dev, DS18B20_CMD_READ_SCRATCHPAD);

    uint8_t temp_lsb = DS18B20_ReadByte(dev);
    uint8_t temp_msb = DS18B20_ReadByte(dev);

    // 拼合成 16 位补码温度数据
    int16_t raw_temp = (int16_t)((temp_msb << 8) | temp_lsb);

    // 12 位分辨率下，每 1 LSB 对应 0.0625 ℃
    float result = (float)raw_temp * 0.0625f;

    dev->temp_c = result;
    if (temp_c != NULL) {
        *temp_c = result;
    }

    return true;
}