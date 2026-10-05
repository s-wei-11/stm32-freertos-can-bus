#include "delay.h"
#include "cmsis_gcc.h"
#include "stm32f407xx.h"
#include <stdint.h>


void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable DWT
    DWT->CYCCNT = 0; // Reset the cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Enable the cycle counter
}

void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    while ((DWT->CYCCNT - start) < cycles);
}

void delay_10ns()
{
    
}

/**
 * @brief ns用nop阻塞延时
 * 80mhz 则为1/80 000 000=80 000/ms 80/1000ns  8次100ns 一次12.5ns
 * @note 调用一次50ns 但是应该不准 进入函数也是时间
 */
void delay_50ns(uint32_t)
{
    __NOP();__NOP();__NOP();__NOP();
}


/**
 * @brief  基于 SysTick 寄存器轮询的微秒级延时
 * @param  us: 需要延时的微秒数
 * @note   适用前提：必须确保 HAL_Init() 已执行，SysTick 已经开始 1ms 倒数
 */
void delay_us_systick(uint32_t us)
{
    // 1. 算总账：算出延时目标需要消耗多少个 CPU 节拍
    // (SystemCoreClock 通常为 168000000)
    uint32_t target_ticks = us * (SystemCoreClock / 1000000); 
    uint32_t elapsed_ticks = 0; // 记录目前已经死等了多少节拍
    uint32_t t_start, t_now;
    // 提取重装载值，备用（应对翻转陷阱）
    uint32_t reload = SysTick->LOAD; 
    // 2. 记录起跑线：看一眼当前的秒表
    t_start = SysTick->VAL; 
    // 3. 开始死等循环
    while (1) 
    {
        // 不断偷看当前的秒表
        t_now = SysTick->VAL; 
        if (t_now != t_start) // 如果时间有流动
        {
            // 分支 A：正常情况（当前值比起始值小，说明在平稳倒数）
            if (t_now < t_start) 
            {
                elapsed_ticks += (t_start - t_now);
            }
            // 分支 B：陷阱情况（当前值比起始值大，说明刚才碰到底部反弹了！）
            else 
            {
                // 走完到底部的路程 (t_start) + 从顶部往下走的路程 (reload - t_now)
                elapsed_ticks += (t_start + (reload - t_now));
            }
            // 更新起跑线，用于下一轮循环累加
            t_start = t_now; 
            // 4. 终点判定：如果累计消耗的节拍达标了，立刻砸碎循环跳出
            if (elapsed_ticks >= target_ticks)break;
        }
    }
}