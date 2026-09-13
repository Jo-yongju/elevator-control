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
#include "stm32g4xx_hal.h"

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
#define HALL_2_Pin GPIO_PIN_0
#define HALL_2_GPIO_Port GPIOC
#define HALL_1_Pin GPIO_PIN_1
#define HALL_1_GPIO_Port GPIOC
#define SEG_B_Pin GPIO_PIN_0
#define SEG_B_GPIO_Port GPIOA
#define SEG_F_Pin GPIO_PIN_1
#define SEG_F_GPIO_Port GPIOA
#define SEG_A_Pin GPIO_PIN_4
#define SEG_A_GPIO_Port GPIOA
#define SEG_D_Pin GPIO_PIN_6
#define SEG_D_GPIO_Port GPIOA
#define SEG_E_Pin GPIO_PIN_7
#define SEG_E_GPIO_Port GPIOA
#define KEY_ROW2_Pin GPIO_PIN_4
#define KEY_ROW2_GPIO_Port GPIOC
#define KEY_ROW1_Pin GPIO_PIN_5
#define KEY_ROW1_GPIO_Port GPIOC
#define KEY_COL3_Pin GPIO_PIN_10
#define KEY_COL3_GPIO_Port GPIOB
#define HALL_4_Pin GPIO_PIN_6
#define HALL_4_GPIO_Port GPIOC
#define SERVO_PWM_Pin GPIO_PIN_7
#define SERVO_PWM_GPIO_Port GPIOC
#define KEY_COL4_Pin GPIO_PIN_8
#define KEY_COL4_GPIO_Port GPIOA
#define HALL_3_Pin GPIO_PIN_9
#define HALL_3_GPIO_Port GPIOA
#define KEY_ROW3_Pin GPIO_PIN_10
#define KEY_ROW3_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_12
#define BUZZER_GPIO_Port GPIOA
#define HALL_5_Pin GPIO_PIN_10
#define HALL_5_GPIO_Port GPIOC
#define KEY_ROW4_Pin GPIO_PIN_3
#define KEY_ROW4_GPIO_Port GPIOB
#define KEY_COL2_Pin GPIO_PIN_4
#define KEY_COL2_GPIO_Port GPIOB
#define KEY_COL1_Pin GPIO_PIN_5
#define KEY_COL1_GPIO_Port GPIOB
#define SEG_DP_Pin GPIO_PIN_6
#define SEG_DP_GPIO_Port GPIOB
#define SEG_G_Pin GPIO_PIN_8
#define SEG_G_GPIO_Port GPIOB
#define SEG_C_Pin GPIO_PIN_9
#define SEG_C_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
