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
 *     <Your Name> - NUCLEO-F429ZI console I/O.
 */

#include "board_init.h"

#include <errno.h>

/**
 * @brief  Retargets the C library printf function to USART3.
 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  Retargets the C library scanf function to USART3.
 */
int __io_getchar(void)
{
    uint8_t ch = 0;
    HAL_UART_Receive(&UartHandle, &ch, 1, HAL_MAX_DELAY);
    /* Echo the character back */
    HAL_UART_Transmit(&UartHandle, &ch, 1, HAL_MAX_DELAY);
    return (int)ch;
}

/**
 * @brief  Newlib _write syscall — routes to USART3 via HAL.
 */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&UartHandle, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief  Newlib _read syscall — reads from USART3.
 */
int _read(int file, char *ptr, int len)
{
    (void)file;
    if (len <= 0)
    {
        return 0;
    }
    /* Read a single character at a time */
    HAL_UART_Receive(&UartHandle, (uint8_t *)ptr, 1, HAL_MAX_DELAY);
    return 1;
}
