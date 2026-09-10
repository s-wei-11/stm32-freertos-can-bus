/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_14
#define LED_GPIO_Port GPIOC
#define LEDC15_Pin GPIO_PIN_15
#define LEDC15_GPIO_Port GPIOC
#define BUTTON_Pin GPIO_PIN_13
#define BUTTON_GPIO_Port GPIOB
#define BUTTONB14_Pin GPIO_PIN_14
#define BUTTONB14_GPIO_Port GPIOB
#define BUTTONB15_Pin GPIO_PIN_15
#define BUTTONB15_GPIO_Port GPIOB
#define CSB_ECH_Pin GPIO_PIN_3
#define CSB_ECH_GPIO_Port GPIOB
#define CSB_TRIG_Pin GPIO_PIN_4
#define CSB_TRIG_GPIO_Port GPIOB
#define LCD_RES_Pin GPIO_PIN_5
#define LCD_RES_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_6
#define LCD_DC_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_7
#define LCD_CS_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_PIN_8
#define LCD_BLK_GPIO_Port GPIOB
#define DS18B20_Pin GPIO_PIN_9
#define DS18B20_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
