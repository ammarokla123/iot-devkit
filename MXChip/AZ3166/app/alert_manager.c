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
 *     <Your Name> - Alert manager for Environmental Monitoring demo.
 */

#include "alert_manager.h"
#include "sensor_monitor.h"
#include "board_init.h"

#include <stdio.h>

/* RGB intensity values (0–255 via PWM) */
#define RGB_OFF     0
#define RGB_DIM     40
#define RGB_BRIGHT  200

/* Blink half-period in ticks (250 ms) */
#define BLINK_HALF_PERIOD  25

/* ------------------------------------------------------------------
   Alert → LED mapping
     No alerts:   solid green,  all status LEDs off
     Warning:     solid yellow, WIFI LED blinks
     Critical:    solid red,    WIFI + USER LEDs blink
   ------------------------------------------------------------------ */

typedef enum
{
    ALERT_LEVEL_NORMAL  = 0,
    ALERT_LEVEL_WARNING = 1,
    ALERT_LEVEL_CRITICAL = 2
} alert_level_t;

static alert_level_t classify_alerts(ULONG flags)
{
    ULONG count = 0;

    if (flags & ALERT_FLAG_TEMP_HIGH)     count++;
    if (flags & ALERT_FLAG_TEMP_LOW)      count++;
    if (flags & ALERT_FLAG_HUMIDITY_HIGH) count++;
    if (flags & ALERT_FLAG_PRESSURE_LOW)  count++;

    if (count >= 2) return ALERT_LEVEL_CRITICAL;
    if (count == 1) return ALERT_LEVEL_WARNING;
    return ALERT_LEVEL_NORMAL;
}

static void set_rgb_for_level(alert_level_t level)
{
    switch (level)
    {
        case ALERT_LEVEL_NORMAL:
            RGB_LED_SET_R(RGB_OFF);
            RGB_LED_SET_G(RGB_DIM);
            RGB_LED_SET_B(RGB_OFF);
            break;
        case ALERT_LEVEL_WARNING:
            RGB_LED_SET_R(RGB_BRIGHT);
            RGB_LED_SET_G(RGB_DIM);
            RGB_LED_SET_B(RGB_OFF);
            break;
        case ALERT_LEVEL_CRITICAL:
            RGB_LED_SET_R(RGB_BRIGHT);
            RGB_LED_SET_G(RGB_OFF);
            RGB_LED_SET_B(RGB_OFF);
            break;
    }
}

static void log_active_alerts(ULONG flags)
{
    if (flags & ALERT_FLAG_TEMP_HIGH)
        printf("ALERT: Temperature above %.1fC\r\n", (double)ALERT_TEMP_HIGH_C);
    if (flags & ALERT_FLAG_TEMP_LOW)
        printf("ALERT: Temperature below %.1fC\r\n", (double)ALERT_TEMP_LOW_C);
    if (flags & ALERT_FLAG_HUMIDITY_HIGH)
        printf("ALERT: Humidity above %.1f%%\r\n", (double)ALERT_HUMIDITY_HIGH_PCT);
    if (flags & ALERT_FLAG_PRESSURE_LOW)
        printf("ALERT: Pressure below %.1f hPa\r\n", (double)ALERT_PRESSURE_LOW_HPA);
}

/* ------------------------------------------------------------------
   Thread entry
   ------------------------------------------------------------------ */
void alert_manager_thread_entry(ULONG parameter)
{
    (void)parameter;

    ULONG actual_flags;
    int   blink_state = 0;

    printf("Alert manager: started\r\n");

    /* Start with green — system healthy */
    RGB_LED_SET_R(RGB_OFF);
    RGB_LED_SET_G(RGB_DIM);
    RGB_LED_SET_B(RGB_OFF);
    WIFI_LED_OFF();
    USER_LED_OFF();

    for (;;)
    {
        /* Wait for any alert flag to be set, or wake periodically to
           clear LEDs if alerts have been resolved.  TX_OR means we wake
           when *any* of the flags are set.  We do NOT consume the flags
           (TX_OR, not TX_OR_CLEAR) because the sensor thread owns them. */
        UINT status = tx_event_flags_get(&alert_flags,
                                         ALERT_FLAG_ALL,
                                         TX_OR,
                                         &actual_flags,
                                         BLINK_HALF_PERIOD);

        if (status == TX_SUCCESS)
        {
            /* At least one alert is active */
            alert_level_t level = classify_alerts(actual_flags);

            set_rgb_for_level(level);
            log_active_alerts(actual_flags);

            /* Blink status LEDs based on severity */
            blink_state = !blink_state;
            if (level >= ALERT_LEVEL_WARNING)
            {
                if (blink_state) WIFI_LED_ON(); else WIFI_LED_OFF();
            }
            if (level >= ALERT_LEVEL_CRITICAL)
            {
                if (blink_state) USER_LED_ON(); else USER_LED_OFF();
            }
        }
        else
        {
            /* Timeout — no alerts, return to normal */
            set_rgb_for_level(ALERT_LEVEL_NORMAL);
            WIFI_LED_OFF();
            USER_LED_OFF();
            blink_state = 0;
        }
    }
}
