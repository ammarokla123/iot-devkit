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
 *     <Your Name> - Display manager for Environmental Monitoring demo.
 */

#ifndef _DISPLAY_MANAGER_H
#define _DISPLAY_MANAGER_H

#include "tx_api.h"

/* Number of display pages the user can cycle through */
#define DISPLAY_PAGE_COUNT  4

typedef enum
{
    DISPLAY_PAGE_CLIMATE  = 0,   /* Temperature + Humidity */
    DISPLAY_PAGE_PRESSURE = 1,   /* Barometric pressure    */
    DISPLAY_PAGE_MOTION   = 2,   /* Accelerometer + Gyro   */
    DISPLAY_PAGE_COMPASS  = 3    /* Magnetometer           */
} display_page_t;

/* Thread entry point */
void display_manager_thread_entry(ULONG parameter);

#endif /* _DISPLAY_MANAGER_H */
