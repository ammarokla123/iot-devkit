#  Copyright (c) 2024 Eclipse Foundation
# 
#  This program and the accompanying materials are made available 
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
# 
#  SPDX-License-Identifier: MIT
# 
#  Contributors: 
#     <Your Name> - NUCLEO-F429ZI application build.

set(SOURCES
    startup/system_stm32f4xx.c
    startup/startup_stm32f429xx.s
    startup/tx_initialize_low_level.S
    stm32cubef4/stm32f4xx_hal_msp.c
    board_init.c
    console.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo

    stm32cubef4
    app_common
    jsmn
    nanoprintf
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
)

target_link_directories(${PROJECT_NAME}
    PUBLIC
        startup
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    message(FATAL_ERROR, "IAR is not currently implemented for this device")
else()
    set_target_linker(${PROJECT_NAME} NUCLEO_F429ZI.ld)
endif()

post_build(${PROJECT_NAME})
