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
/* clang-format off */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_dma.h"

#include "stm32f1xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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
#define HC595_RCLK_Pin GPIO_PIN_4
#define HC595_RCLK_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_12
#define LED_GPIO_Port GPIOB
#define BUTTON_Pin GPIO_PIN_13
#define BUTTON_GPIO_Port GPIOB
#define BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define PUMP_Pin GPIO_PIN_14
#define PUMP_GPIO_Port GPIOB
#define Beep_Pin GPIO_PIN_8
#define Beep_GPIO_Port GPIOA
#define ENC_KEY_Pin GPIO_PIN_3
#define ENC_KEY_GPIO_Port GPIOB
#define ENC_KEY_EXTI_IRQn EXTI3_IRQn
#define ENC_A_Pin GPIO_PIN_4
#define ENC_A_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_5
#define ENC_B_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* clang-format on */
#define BEEP_USE_NO_SOURCE 1

#if BEEP_USE_NO_SOURCE
void beep_on(void);
void beep_off(void);
#define beep_init() MX_TIM1_Init()
void beep_toggle(void);
#else /* BEEP_USE_NO_SOURCE */
void beep_init(void);
#define beep_on()     HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET)
#define beep_off()    HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET)
#define beep_toggle() HAL_GPIO_TogglePin(Beep_GPIO_Port, Beep_Pin)
#endif /* BEEP_USE_NO_SOURCE */

#define LED_ON()     (LED_GPIO_Port->BRR = LED_Pin)
#define LED_OFF()    (LED_GPIO_Port->BSRR = LED_Pin)
#define LED_UPDATE() (PUMP_IS_ON() ? LED_ON() : LED_OFF())

#define PUMP_ON()    (PUMP_GPIO_Port->BRR = PUMP_Pin)
#define PUMP_OFF()   (PUMP_GPIO_Port->BSRR = PUMP_Pin);
#define PUMP_IS_ON() (HAL_GPIO_ReadPin(PUMP_GPIO_Port, PUMP_Pin) == GPIO_PIN_RESET ? 1U : 0U)
#define PUMP_TOGGLE()                                 \
    do {                                              \
        HAL_GPIO_TogglePin(PUMP_GPIO_Port, PUMP_Pin); \
        LED_UPDATE();                                 \
    } while (0)

#define BUTTON_IRQ_ENABLE()   HAL_NVIC_EnableIRQ(EXTI3_IRQn)
#define BUTTON_IRQ_DISABLE()  HAL_NVIC_DisableIRQ(EXTI3_IRQn)

#define ENC_KEY_IRQ_ENABLE()  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)
#define ENC_KEY_IRQ_DISABLE() HAL_NVIC_DisableIRQ(EXTI15_10_IRQn)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
