#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include "ds18b20.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "semphr.h"
#include "stdarg.h"


//放到eeprom里面去 第一次肯定没有 第二读到55aa说明里面设定过了
#define NODE2_MAGIC_NUM  0x55AA  // EEPROM 初始化有效标记 



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
    uint16_t magic_num; //校验值 用来判定eeprom是否设定过阈值
    //参数阈值
    uint8_t temp_lev1_thresh;
    uint8_t temp_lev2_thresh;
    uint8_t temp_lev3_thresh;

    //状态超时驻留
    uint16_t timeout_lev2_s;    //2级温度持续不下降时间
    uint16_t timeout_lev3_s;    //3级时间

    //滞回阈值 触发点不变 — 解除点回差
    uint8_t hys_val;    //温度滞回阈值
}__attribute__((packed)) node2_threshold;   //字节不留空隙



//填充需要传出的数据
typedef struct
{
    uint16_t current_temp; //当前温度   温度放大10倍
    bool     rain_state;       //雨水状态

    node_mode_t work_mode;  //工作状态
    temp_state  temp_level; //温度等级

    uint8_t louver_target_percent;  //步进电机 目标角度
    uint8_t louver_current_percent; //步进电机 当前角度
    uint8_t fan_pwm_precent;    //风扇占空比状况

}node2_total_state;

//逻辑判断 从

extern node2_total_state node2_state; //定义总状态对象
extern node2_threshold   threshold_one; //at24c02阈值存储对象



void node2_device_init();           //设备初始化
void node2_apply_outputs(void);     //设备（电机步进电机状态更新）
void node2_state_update(node2_total_state * dev);   //温度状态更新


// 电机任务事件通知掩码（Bit 0: 紧急打断并强制归零关窗）
#define MOTOR_SIG_ABORT_TO_ZERO   (1UL << 0)            //表示有雨 事件通知值
#define MOTOR_SIG_START_MOVE      (1UL << 1)            // Bit 1: 目标开度更新，唤醒电机干活


void Stepper_Louver_Control(node2_total_state *obj); //步进电机控制执行
void Stepper_Zero_Calibrate(void);  //步进电机初始化
void get_temp(node2_total_state * dev,Ds18bxx_t * ds18b20_t);



void sys_log_init();
void safe_printf(const char *format, ...);

