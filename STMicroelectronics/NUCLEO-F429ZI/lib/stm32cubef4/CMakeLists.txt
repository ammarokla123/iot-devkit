#  Copyright (c) 2024 Eclipse Foundation
# 
#  This program and the accompanying materials are made available 
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
# 
#  SPDX-License-Identifier: MIT
# 
#  Contributors: 
#     <Your Name> - Initial NUCLEO-F429ZI board support.

# The NUCLEO-F429ZI uses the same STM32CubeF4 HAL as the MXChip AZ3166.
# The HAL driver sources are shared across all STM32F4 variants.
# We reference the MXChip's copy for HAL sources, CMSIS common headers,
# and HAL headers.  The F429-specific CMSIS device header is provided
# locally in our own Drivers/CMSIS/Device/ tree.

set(STM32_FAMILY "F4")

# Point to MXChip's stm32cubef4 for HAL/CMSIS common sources
set(MXCHIP_CUBEF4_DIR "${CMAKE_SOURCE_DIR}/../../MXChip/AZ3166/lib/stm32cubef4")
set(STM32Cube_DIR "${MXCHIP_CUBEF4_DIR}")

find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS dma eth flash gpio tim uart usart)

set(TARGET stm32cubef4)

add_library(${TARGET} OBJECT
    ${STM32HAL_SOURCES})

target_compile_definitions(${TARGET}
    PUBLIC
        STM32F429xx
        USE_HAL_DRIVER
        STM32F4
)

target_include_directories(${TARGET}
    PUBLIC
        ${CMSIS_COMMON_INCLUDE_DIR}
        ${STM32HAL_INCLUDE_DIR}
        # Use our own device include (has stm32f429xx.h) and hal_conf
        ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
        config
)
