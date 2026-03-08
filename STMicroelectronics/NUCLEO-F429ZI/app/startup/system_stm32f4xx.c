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
 *     <Your Name> - NUCLEO-F429ZI system initialisation.
 */

/**
 * System clock configuration for NUCLEO-F429ZI:
 *
 *   HSE input:    8 MHz (ST-LINK MCO bypass)
 *   PLL_M = 8     → VCO input  = 1 MHz
 *   PLL_N = 360   → VCO output = 360 MHz
 *   PLL_P = 2     → SYSCLK     = 180 MHz
 *   PLL_Q = 7     → USB clock  ≈ 51.4 MHz (close enough for non-USB use)
 *
 *   AHB  prescaler = 1  → HCLK  = 180 MHz
 *   APB1 prescaler = 4  → PCLK1 = 45 MHz
 *   APB2 prescaler = 2  → PCLK2 = 90 MHz
 *   Flash latency  = 5 wait states (for 180 MHz @ 3.3 V)
 */

#include "stm32f4xx.h"

/* PLL parameters */
#define PLL_M      8
#define PLL_N      360
#define PLL_P      2
#define PLL_Q      7

uint32_t SystemCoreClock = 180000000U;

const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

void SystemInit(void)
{
    /* FPU settings — enable CP10 and CP11 full access */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
#endif

    /* Reset the RCC clock configuration to the default reset state */
    RCC->CR |= (uint32_t)0x00000001;   /* Set HSION bit           */
    RCC->CFGR = 0x00000000;            /* Reset CFGR register     */
    RCC->CR &= (uint32_t)0xFEF6FFFF;   /* Reset HSEON, CSSON, PLLON */
    RCC->PLLCFGR = 0x24003010;         /* Reset PLLCFGR register  */
    RCC->CR &= (uint32_t)0xFFFBFFFF;   /* Reset HSEBYP bit       */
    RCC->CIR = 0x00000000;             /* Disable all interrupts  */

    /* Configure the Vector Table location — Flash base */
    SCB->VTOR = FLASH_BASE;
}

void SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

    /* Get SYSCLK source */
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
        case 0x00:  /* HSI */
            SystemCoreClock = 16000000U;
            break;
        case 0x04:  /* HSE */
            SystemCoreClock = HSE_VALUE;
            break;
        case 0x08:  /* PLL */
            pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
            pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

            if (pllsource != 0)
            {
                pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
            }
            else
            {
                pllvco = (16000000U / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
            }

            pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> 16) + 1) * 2;
            SystemCoreClock = pllvco / pllp;
            break;
        default:
            SystemCoreClock = 16000000U;
            break;
    }

    /* Compute HCLK frequency */
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    SystemCoreClock >>= tmp;
}
