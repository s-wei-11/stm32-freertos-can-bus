/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Cont_Task */
osThreadId_t Cont_TaskHandle;
const osThreadAttr_t Cont_Task_attributes = {
  .name = "Cont_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UI_Task */
osThreadId_t UI_TaskHandle;
const osThreadAttr_t UI_Task_attributes = {
  .name = "UI_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Receive_Task */
osThreadId_t Receive_TaskHandle;
const osThreadAttr_t Receive_Task_attributes = {
  .name = "Receive_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Process_Task */
osThreadId_t Process_TaskHandle;
const osThreadAttr_t Process_Task_attributes = {
  .name = "Process_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void vCont_task(void *argument);
void UI_display_task(void *argument);
void data_acquire(void *argument);
void vprocess_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   /* 一旦任务切换检测到栈溢出，内核会主动跳进这里 */
   /* 1. 先用 volatile 变量存下名字，方便仿真器直观查看 */
    volatile signed char *overflow_task = pcTaskName;
    (void)overflow_task;
    (void)xTask;          //这样是为了方便溢出串口无法工作 通过断点查看 
    printf("发生栈溢出！任务名: %s\r\n", pcTaskName);
    __disable_irq(); // 关全局中断，冻结案发现场
    while(1);        // 在这里打断点
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Cont_Task */
  Cont_TaskHandle = osThreadNew(vCont_task, NULL, &Cont_Task_attributes);

  /* creation of UI_Task */
  UI_TaskHandle = osThreadNew(UI_display_task, NULL, &UI_Task_attributes);

  /* creation of Receive_Task */
  Receive_TaskHandle = osThreadNew(data_acquire, NULL, &Receive_Task_attributes);

  /* creation of Process_Task */
  Process_TaskHandle = osThreadNew(vprocess_task, NULL, &Process_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vCont_task */
/**
  * @brief  Function implementing the Cont_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vCont_task */
__weak void vCont_task(void *argument)
{
  /* USER CODE BEGIN vCont_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vCont_task */
}

/* USER CODE BEGIN Header_UI_display_task */
/**
* @brief Function implementing the UI_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UI_display_task */
__weak void UI_display_task(void *argument)
{
  /* USER CODE BEGIN UI_display_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END UI_display_task */
}

/* USER CODE BEGIN Header_data_acquire */
/**
* @brief Function implementing the Receive_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_data_acquire */
__weak void data_acquire(void *argument)
{
  /* USER CODE BEGIN data_acquire */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END data_acquire */
}

/* USER CODE BEGIN Header_vprocess_task */
/**
* @brief Function implementing the Process_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vprocess_task */
__weak void vprocess_task(void *argument)
{
  /* USER CODE BEGIN vprocess_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vprocess_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

