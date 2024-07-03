/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

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
#define IN_KEY3_Pin GPIO_PIN_13
#define IN_KEY3_GPIO_Port GPIOC
#define IN_KEY2_Pin GPIO_PIN_14
#define IN_KEY2_GPIO_Port GPIOC
#define IN_KEY1_Pin GPIO_PIN_15
#define IN_KEY1_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_0
#define LED3_GPIO_Port GPIOF
#define LED4_Pin GPIO_PIN_1
#define LED4_GPIO_Port GPIOF
#define LP_nReset_Pin GPIO_PIN_10
#define LP_nReset_GPIO_Port GPIOB
#define LP_nRead_Pin GPIO_PIN_11
#define LP_nRead_GPIO_Port GPIOB
#define ESP_EN_Pin GPIO_PIN_12
#define ESP_EN_GPIO_Port GPIOB
#define FG2_Pin GPIO_PIN_0
#define FG2_GPIO_Port GPIOD
#define FR2_Pin GPIO_PIN_1
#define FR2_GPIO_Port GPIOD
#define FR1_Pin GPIO_PIN_2
#define FR1_GPIO_Port GPIOD
#define FG1_Pin GPIO_PIN_3
#define FG1_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
