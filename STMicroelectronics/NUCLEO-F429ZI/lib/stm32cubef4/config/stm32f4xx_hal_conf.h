/**
  ******************************************************************************
  * @file    stm32f4xx_hal_conf.h
  * @brief   HAL configuration for STM32 NUCLEO-F429ZI board.
  *
  * SPDX-License-Identifier: BSD-3-Clause
  ******************************************************************************
  */

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_ETH_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED

/* ########################## HSE/HSI Values adaptation ##################### */
/* NUCLEO-F429ZI uses an 8 MHz HSE bypass from the ST-LINK MCO output */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE              8000000U
#endif

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U
#endif

#if !defined  (HSI_VALUE)
  #define HSI_VALUE              16000000U
#endif

#if !defined  (LSI_VALUE)
  #define LSI_VALUE              32000U
#endif

#if !defined  (LSE_VALUE)
  #define LSE_VALUE              32768U
#endif

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U
#endif

#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE   12288000U
#endif

/* ########################### System Configuration ######################### */
#define  VDD_VALUE                    3300U
#define  TICK_INT_PRIORITY            0x0FU
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U
#define  INSTRUCTION_CACHE_ENABLE     1U
#define  DATA_CACHE_ENABLE            1U

/* ########################## Assert Selection ############################## */
/* Uncomment to enable HAL assert checking */
/* #define USE_FULL_ASSERT    1U */

/* ################## Ethernet peripheral configuration ##################### */
/* LAN8742A PHY Address — default for NUCLEO-F429ZI Ethernet */
#define LAN8742A_PHY_ADDRESS          0x00U

/* Ethernet MAC/DMA configuration */
#define ETH_RX_BUF_SIZE              ETH_MAX_PACKET_SIZE
#define ETH_TX_BUF_SIZE              ETH_MAX_PACKET_SIZE
#define ETH_RXBUFNB                  4U
#define ETH_TXBUFNB                  4U

/* DP83848 PHY register values (also compatible with LAN8742A) */
#define PHY_BCR                       0x00U    /* Basic Control Register */
#define PHY_RESET                     0x8000U
#define PHY_LOOPBACK                  0x4000U
#define PHY_FULLDUPLEX_100M           0x2100U
#define PHY_HALFDUPLEX_100M           0x2000U
#define PHY_AUTONEGOTIATION           0x1000U
#define PHY_RESTART_AUTONEGOTIATION   0x0200U
#define PHY_POWERDOWN                 0x0800U
#define PHY_ISOLATE                   0x0400U
#define PHY_AUTONEGO_COMPLETE         0x0020U
#define PHY_LINKED_STATUS             0x0004U
#define PHY_JABBER_DETECTION          0x0002U

#define PHY_BSR                       0x01U
#define PHY_SR                        0x1FU    /* LAN8742A special status register */
#define PHY_SPEED_STATUS              0x0004U
#define PHY_DUPLEX_STATUS             0x0010U

/* PHY delay and timeout values */
#define PHY_RESET_DELAY               0x000000FFU
#define PHY_CONFIG_DELAY              0x00000FFFU
#define PHY_READ_TO                   0x0000FFFFU
#define PHY_WRITE_TO                  0x0000FFFFU

/* ################## SPI peripheral configuration ########################## */
#define USE_SPI_CRC                   0U

/* Includes ----------------------------------------------------------------- */
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32f4xx_hal_exti.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif

#ifdef HAL_ETH_MODULE_ENABLED
  #include "stm32f4xx_hal_eth.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f4xx_hal_pwr.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f4xx_hal_tim.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
  #include "stm32f4xx_hal_uart.h"
#endif

#ifdef HAL_USART_MODULE_ENABLED
  #include "stm32f4xx_hal_usart.h"
#endif

/* ########################## Assert Selection ############################## */
#ifdef USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
