/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_gpio.h"

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
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define UART2_TX_Pin LL_GPIO_PIN_2
#define UART2_TX_GPIO_Port GPIOA
#define UART2_RX_Pin LL_GPIO_PIN_3
#define UART2_RX_GPIO_Port GPIOA
#define LD2_Pin LL_GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define ID0_Pin LL_GPIO_PIN_0
#define ID0_GPIO_Port GPIOB
#define ID1_Pin LL_GPIO_PIN_1
#define ID1_GPIO_Port GPIOB
#define ID2_Pin LL_GPIO_PIN_2
#define ID2_GPIO_Port GPIOB
#define UART3_TX_Pin LL_GPIO_PIN_10
#define UART3_TX_GPIO_Port GPIOB
#define UART3_RX_Pin LL_GPIO_PIN_11
#define UART3_RX_GPIO_Port GPIOB
#define UART1_TX_Pin LL_GPIO_PIN_9
#define UART1_TX_GPIO_Port GPIOA
#define UART1_RX_Pin LL_GPIO_PIN_10
#define UART1_RX_GPIO_Port GPIOA
#define T_SWDIO_Pin LL_GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin LL_GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define UART4_TX_Pin LL_GPIO_PIN_10
#define UART4_TX_GPIO_Port GPIOC
#define UART4_RX_Pin LL_GPIO_PIN_11
#define UART4_RX_GPIO_Port GPIOC
#define UART5_TX_Pin LL_GPIO_PIN_12
#define UART5_TX_GPIO_Port GPIOC
#define UART5_RX_Pin LL_GPIO_PIN_2
#define UART5_RX_GPIO_Port GPIOD
#define T_SWO_Pin LL_GPIO_PIN_3
#define T_SWO_GPIO_Port GPIOB
#define USART1_MODE_Pin LL_GPIO_PIN_4
#define USART1_MODE_GPIO_Port GPIOB
#define USART2_MODE_Pin LL_GPIO_PIN_5
#define USART2_MODE_GPIO_Port GPIOB
#define USART3_MODE_Pin LL_GPIO_PIN_6
#define USART3_MODE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
