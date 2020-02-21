/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifdef __cplusplus
}
#endif
#include "user.h"
#ifdef __cplusplus
extern "C" {
#endif
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
#define ROTARYB_Pin GPIO_PIN_0
#define ROTARYB_GPIO_Port GPIOA
#define ROTARYB_EXTI_IRQn EXTI0_1_IRQn
#define ROTARYA_Pin GPIO_PIN_1
#define ROTARYA_GPIO_Port GPIOA
#define ROTARYA_EXTI_IRQn EXTI0_1_IRQn
#define MOSFET_Pin GPIO_PIN_2
#define MOSFET_GPIO_Port GPIOA
#define DEBUG2_Pin GPIO_PIN_3
#define DEBUG2_GPIO_Port GPIOA
#define DEBUG1_Pin GPIO_PIN_6
#define DEBUG1_GPIO_Port GPIOA
#define BTN_Pin GPIO_PIN_9
#define BTN_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
