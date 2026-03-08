/* 
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     <Your Name> - NUCLEO-F429ZI board support.
 */

#ifndef _BOARD_INIT_H
#define _BOARD_INIT_H

#include "stm32f4xx_hal.h"

/* NUCLEO-F429ZI user LEDs:
 *   LD1 (green)  — PB0
 *   LD2 (blue)   — PB7
 *   LD3 (red)    — PB14
 */
#define LED1_PIN  GPIO_PIN_0
#define LED2_PIN  GPIO_PIN_7
#define LED3_PIN  GPIO_PIN_14
#define LED_PORT  GPIOB

#define LED1_ON()   HAL_GPIO_WritePin(LED_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_OFF()  HAL_GPIO_WritePin(LED_PORT, LED1_PIN, GPIO_PIN_RESET)

#define LED2_ON()   HAL_GPIO_WritePin(LED_PORT, LED2_PIN, GPIO_PIN_SET)
#define LED2_OFF()  HAL_GPIO_WritePin(LED_PORT, LED2_PIN, GPIO_PIN_RESET)

#define LED3_ON()   HAL_GPIO_WritePin(LED_PORT, LED3_PIN, GPIO_PIN_SET)
#define LED3_OFF()  HAL_GPIO_WritePin(LED_PORT, LED3_PIN, GPIO_PIN_RESET)

#define LED1_TOGGLE()  HAL_GPIO_TogglePin(LED_PORT, LED1_PIN)
#define LED2_TOGGLE()  HAL_GPIO_TogglePin(LED_PORT, LED2_PIN)
#define LED3_TOGGLE()  HAL_GPIO_TogglePin(LED_PORT, LED3_PIN)

/* User button — PC13 (active high on NUCLEO-F429ZI) */
#define USER_BUTTON_PIN   GPIO_PIN_13
#define USER_BUTTON_PORT  GPIOC
#define USER_BUTTON_IS_PRESSED  (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_SET)

extern UART_HandleTypeDef UartHandle;

void board_init(void);

#endif /* _BOARD_INIT_H */
