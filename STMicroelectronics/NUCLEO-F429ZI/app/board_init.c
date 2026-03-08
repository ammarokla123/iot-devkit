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

#include "board_init.h"
#include <stdio.h>

UART_HandleTypeDef UartHandle;

/* ------------------------------------------------------------------ */
/* Private function prototypes                                        */
/* ------------------------------------------------------------------ */
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART_Console_Init(void);
static void Error_Handler(void);

/* ------------------------------------------------------------------ */
/* SystemClock_Config — HSE 8 MHz (bypass) → PLL → 180 MHz SYSCLK    */
/* ------------------------------------------------------------------ */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when
       the device is clocked below the maximum system frequency. */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE in bypass mode (8 MHz from ST-LINK MCO) */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 8;
    RCC_OscInitStruct.PLL.PLLN       = 360;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;   /* 180 MHz */
    RCC_OscInitStruct.PLL.PLLQ       = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Enable over-drive mode for 180 MHz operation */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }

    /* Select PLL as system clock source and configure AHB/APB prescalers */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;     /* 180 MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;       /*  45 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;       /*  90 MHz */

    /* 5 wait states for 180 MHz at 3.3 V */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ------------------------------------------------------------------ */
/* GPIO_Init — LEDs (PB0, PB7, PB14) and user button (PC13)          */
/* ------------------------------------------------------------------ */
static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure LED pins as push-pull output */
    GPIO_InitStruct.Pin   = LED1_PIN | LED2_PIN | LED3_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /* All LEDs off by default */
    HAL_GPIO_WritePin(LED_PORT, LED1_PIN | LED2_PIN | LED3_PIN, GPIO_PIN_RESET);

    /* Configure user button as input */
    GPIO_InitStruct.Pin  = USER_BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USER_BUTTON_PORT, &GPIO_InitStruct);
}

/* ------------------------------------------------------------------ */
/* UART_Console_Init — USART3 @ 115200 baud (ST-LINK VCP)            */
/*   TX = PD8, RX = PD9 (configured in HAL MSP)                      */
/* ------------------------------------------------------------------ */
static void UART_Console_Init(void)
{
    UartHandle.Instance          = USART3;
    UartHandle.Init.BaudRate     = 115200;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.HwFlowCtl   = UART_HWCONTROL_NONE;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ------------------------------------------------------------------ */
/* board_init — called from main() before tx_kernel_enter()           */
/* ------------------------------------------------------------------ */
void board_init(void)
{
    HAL_Init();
    SystemClock_Config();
    UART_Console_Init();
    GPIO_Init();

    printf("\r\n------------------------------------------\r\n");
    printf(" NUCLEO-F429ZI — Eclipse ThreadX Starter\r\n");
    printf(" System clock: 180 MHz\r\n");
    printf("------------------------------------------\r\n\r\n");
}

/* ------------------------------------------------------------------ */
/* Error_Handler — called on unrecoverable init failure               */
/* ------------------------------------------------------------------ */
static void Error_Handler(void)
{
    /* Turn on the red LED and loop forever */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = LED3_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_PORT, LED3_PIN, GPIO_PIN_SET);

    for (;;)
    {
    }
}
