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
 *     <Your Name> - Sensor monitoring module for Environmental Monitoring demo.
 */

#include "sensor_monitor.h"

#include <stdio.h>

#include "sensor.h"
#include "board_init.h"

/* ---------- Kernel objects ---------- */
TX_QUEUE             sensor_data_queue;
TX_EVENT_FLAGS_GROUP alert_flags;
TX_MUTEX             i2c_mutex;
static TX_TIMER      sensor_timer;

/* Queue storage — holds up to 4 sensor snapshots. Each message is
   sizeof(sensor_data_t) / sizeof(ULONG) words. */
#define SENSOR_QUEUE_DEPTH     4
#define SENSOR_MSG_SIZE_WORDS  ((sizeof(sensor_data_t) + sizeof(ULONG) - 1) / sizeof(ULONG))
static ULONG sensor_queue_storage[SENSOR_QUEUE_DEPTH * SENSOR_MSG_SIZE_WORDS];

/* Semaphore used by the periodic timer to wake the sensor thread */
static TX_SEMAPHORE sensor_tick_sem;

/* ---------- Timer callback ---------- */
static void sensor_timer_callback(ULONG id)
{
    (void)id;
    tx_semaphore_put(&sensor_tick_sem);
}

/* ---------- Alert evaluation ---------- */
static void evaluate_alerts(const sensor_data_t *data)
{
    ULONG flags_to_set = 0;

    if (data->humidity.temperature_degC > ALERT_TEMP_HIGH_C)
    {
        flags_to_set |= ALERT_FLAG_TEMP_HIGH;
    }

    if (data->humidity.temperature_degC < ALERT_TEMP_LOW_C)
    {
        flags_to_set |= ALERT_FLAG_TEMP_LOW;
    }

    if (data->humidity.humidity_perc > ALERT_HUMIDITY_HIGH_PCT)
    {
        flags_to_set |= ALERT_FLAG_HUMIDITY_HIGH;
    }

    if (data->pressure.pressure_hPa < ALERT_PRESSURE_LOW_HPA)
    {
        flags_to_set |= ALERT_FLAG_PRESSURE_LOW;
    }

    /* Clear all flags first, then set the active ones.  The alert thread
       watches for any flag being set with TX_OR. */
    tx_event_flags_set(&alert_flags, ~ALERT_FLAG_ALL, TX_AND);

    if (flags_to_set != 0)
    {
        tx_event_flags_set(&alert_flags, flags_to_set, TX_OR);
    }
}

/* ---------- Thread entry ---------- */
void sensor_monitor_thread_entry(ULONG parameter)
{
    (void)parameter;

    sensor_data_t reading;

    printf("Sensor monitor: started\r\n");

    /* Start the periodic timer */
    tx_timer_activate(&sensor_timer);

    for (;;)
    {
        /* Sleep until the periodic timer fires */
        tx_semaphore_get(&sensor_tick_sem, TX_WAIT_FOREVER);

        /* Acquire I2C bus (shared with display on I2C1) */
        tx_mutex_get(&i2c_mutex, TX_WAIT_FOREVER);

        reading.pressure      = lps22hb_data_read();
        reading.humidity       = hts221_data_read();
        reading.motion         = lsm6dsl_data_read();
        reading.magnetometer   = lis2mdl_data_read();
        reading.timestamp_ticks = tx_time_get();

        tx_mutex_put(&i2c_mutex);

        /* Evaluate alert thresholds */
        evaluate_alerts(&reading);

        /* Post to queue — if full, drop the oldest by not waiting */
        tx_queue_send(&sensor_data_queue, &reading, TX_NO_WAIT);
    }
}

/* ---------- Initialisation (called from tx_application_define) ---------- */
UINT sensor_monitor_init(TX_BYTE_POOL *byte_pool)
{
    (void)byte_pool;
    UINT status;

    /* Message queue */
    status = tx_queue_create(&sensor_data_queue,
                             "Sensor Queue",
                             SENSOR_MSG_SIZE_WORDS,
                             sensor_queue_storage,
                             sizeof(sensor_queue_storage));
    if (status != TX_SUCCESS) return status;

    /* Event flags for alerts */
    status = tx_event_flags_create(&alert_flags, "Alert Flags");
    if (status != TX_SUCCESS) return status;

    /* Mutex for I2C bus protection */
    status = tx_mutex_create(&i2c_mutex, "I2C Mutex", TX_NO_INHERIT);
    if (status != TX_SUCCESS) return status;

    /* Counting semaphore for timer → thread signalling */
    status = tx_semaphore_create(&sensor_tick_sem, "Sensor Tick", 0);
    if (status != TX_SUCCESS) return status;

    /* Periodic timer — created but not yet activated (started in thread) */
    status = tx_timer_create(&sensor_timer,
                             "Sensor Timer",
                             sensor_timer_callback,
                             0,
                             SENSOR_READ_INTERVAL_TICKS,
                             SENSOR_READ_INTERVAL_TICKS,
                             TX_NO_ACTIVATE);
    return status;
}
