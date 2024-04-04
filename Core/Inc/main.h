/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_BLUE_Pin GPIO_PIN_13
#define B1_BLUE_GPIO_Port GPIOC
#define ECHO_FRONT_US_Pin GPIO_PIN_0
#define ECHO_FRONT_US_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define CENTER_LEFT_TOF_Pin GPIO_PIN_4
#define CENTER_LEFT_TOF_GPIO_Port GPIOC
#define FRONT_LEFT_TOF_Pin GPIO_PIN_5
#define FRONT_LEFT_TOF_GPIO_Port GPIOC
#define TRIGGER_FRONT_US_Pin GPIO_PIN_12
#define TRIGGER_FRONT_US_GPIO_Port GPIOB
#define TRIGGER_BACK_US_Pin GPIO_PIN_13
#define TRIGGER_BACK_US_GPIO_Port GPIOB
#define ECHO_BACK_US_Pin GPIO_PIN_15
#define ECHO_BACK_US_GPIO_Port GPIOB
#define FRONT_RIGHT_TOF_Pin GPIO_PIN_6
#define FRONT_RIGHT_TOF_GPIO_Port GPIOC
#define BACK_LEFT_TOF_Pin GPIO_PIN_7
#define BACK_LEFT_TOF_GPIO_Port GPIOC
#define BACK_RIGHT_TOF_Pin GPIO_PIN_8
#define BACK_RIGHT_TOF_GPIO_Port GPIOC
#define CENTER_RIGHT_TOF_Pin GPIO_PIN_9
#define CENTER_RIGHT_TOF_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
