#pragma once
#include <stdbool.h>
#include <stdint.h>
//节点模式
typedef enum
{
    node2_auto=0,   //自动模式
    node2_manual    //手动模式
}node_mode_t;

//温度各个状态
typedef enum 
{
    normal=0,           //温度正常
    temp_high_lev1,
    temp_high_lev2,
    temp_high_lev3,   //三级高温
    temp_abnormal       //温度异常
}temp_state;


//存于AT24C02
typedef struct{

    //参数阈值
    uint8_t temp_lev1_thresh;
    uint8_t temp_lev2_thresh;
    uint8_t temp_lev3_thresh;

    //状态超时驻留
    uint16_t timeout_lev2_s;    //2级温度持续不下降时间
    uint16_t timeout_lev3_s;    //3级时间

}node2_threshold;



//填充需要传出的数据
typedef struct
{
    uint16_t current_temp; //当前温度   温度放大10倍
    bool  rain_state;

    node_mode_t work_mode;  //工作状态
    temp_state  temp_level; //温度等级

    uint8_t louver_target_percent;  //步进电机 目标角度
    uint8_t louver_current_percent; //步进电机 当前角度
    uint8_t fan_pwm_precent;    //风扇占空比状况


}node2_total_state;




void stepper_control(void);