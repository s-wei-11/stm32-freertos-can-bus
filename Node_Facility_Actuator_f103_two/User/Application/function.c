#include "function.h"
#include "step_28byj48.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include "ds18b20.h"
#include "tim.h"

//雨水检测 1为无雨 0为有雨
#define rain_check (!!(GPIOB->IDR&GPIO_PIN_1))



void node2_device_init()
{
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    TIM3->CCR2=0;//初始为正常状态
}



extern Stepper_28BYJ48_t g_motor;
void stepper_control()
{
   uint16_t  step = (uint16_t )Stepper_AngleToSteps(90);

    if(rain_check)
    {
        Stepper_PowerOff(&g_motor);
    }
    else 
    {
        for(uint16_t k=0;k<step;k++)
        {
            Stepper_Step(&g_motor,STEPPER_DIR_CW);
             HAL_Delay(4);
          
        }
         Stepper_PowerOff(&g_motor);//转完停
    }
}


void dc_motor()
{
    
}