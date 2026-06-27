/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

#define LED_ON()              (LED_GPIO_Port->BRR = LED_Pin)
#define LED_OFF()             (LED_GPIO_Port->BSRR = LED_Pin)
#define LED_UPDATE()          (pump_is_on() ? LED_ON() : LED_OFF())

#define BUTTON_IRQ_ENABLE()   HAL_NVIC_EnableIRQ(EXTI3_IRQn)
#define BUTTON_IRQ_DISABLE()  HAL_NVIC_DisableIRQ(EXTI3_IRQn)

#define ENC_KEY_IRQ_ENABLE()  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)
#define ENC_KEY_IRQ_DISABLE() HAL_NVIC_DisableIRQ(EXTI15_10_IRQn)

extern uint32_t g_pump_on_tick;

void pump_on(void);
void pump_off(void);
uint32_t pump_is_on(void);
void pump_toggle(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */
