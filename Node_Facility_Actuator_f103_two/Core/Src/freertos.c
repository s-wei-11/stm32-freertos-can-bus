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
/* Definitions for APP_TASK */
osThreadId_t APP_TASKHandle;
const osThreadAttr_t APP_TASK_attributes = {
  .name = "APP_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Acquire_Task */
osThreadId_t Acquire_TaskHandle;
const osThreadAttr_t Acquire_Task_attributes = {
  .name = "Acquire_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Button_Queue */
osMessageQueueId_t Button_QueueHandle;
const osMessageQueueAttr_t Button_Queue_attributes = {
  .name = "Button_Queue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void App_Show_Task(void *argument);
void Data_Acquire_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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

  /* Create the queue(s) */
  /* creation of Button_Queue */
  Button_QueueHandle = osMessageQueueNew (8, sizeof(uint8_t), &Button_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of APP_TASK */
  APP_TASKHandle = osThreadNew(App_Show_Task, NULL, &APP_TASK_attributes);

  /* creation of Acquire_Task */
  Acquire_TaskHandle = osThreadNew(Data_Acquire_Task, NULL, &Acquire_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_App_Show_Task */
/**
  * @brief  Function implementing the APP_TASK thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_App_Show_Task */
__weak void App_Show_Task(void *argument)
{
  /* USER CODE BEGIN App_Show_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END App_Show_Task */
}

/* USER CODE BEGIN Header_Data_Acquire_Task */
/**
* @brief Function implementing the Acquire_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Data_Acquire_Task */
__weak void Data_Acquire_Task(void *argument)
{
  /* USER CODE BEGIN Data_Acquire_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Data_Acquire_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

