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

#ifndef _SENSOR_MONITOR_H
#define _SENSOR_MONITOR_H

#include "tx_api.h"
#include "sensor.h"

/* Sensor reading interval in ThreadX timer ticks (100 ticks/sec default).
   500 ticks = 5 seconds between readings. */
#define SENSOR_READ_INTERVAL_TICKS  500

/* Alert threshold defaults — user may override in cloud_config.h */
#ifndef ALERT_TEMP_HIGH_C
#define ALERT_TEMP_HIGH_C           35.0f
#endif

#ifndef ALERT_TEMP_LOW_C
#define ALERT_TEMP_LOW_C            10.0f
#endif

#ifndef ALERT_HUMIDITY_HIGH_PCT
#define ALERT_HUMIDITY_HIGH_PCT     80.0f
#endif

#ifndef ALERT_PRESSURE_LOW_HPA
#define ALERT_PRESSURE_LOW_HPA      980.0f
#endif

/* Event flag bits for alert conditions */
#define ALERT_FLAG_TEMP_HIGH        ((ULONG)0x01)
#define ALERT_FLAG_TEMP_LOW         ((ULONG)0x02)
#define ALERT_FLAG_HUMIDITY_HIGH    ((ULONG)0x04)
#define ALERT_FLAG_PRESSURE_LOW     ((ULONG)0x08)
#define ALERT_FLAG_ALL              (ALERT_FLAG_TEMP_HIGH | ALERT_FLAG_TEMP_LOW | \
                                     ALERT_FLAG_HUMIDITY_HIGH | ALERT_FLAG_PRESSURE_LOW)

/* Consolidated sensor snapshot pushed through the message queue */
typedef struct sensor_data_s
{
    lps22hb_t      pressure;
    hts221_data_t  humidity;
    lsm6dsl_data_t motion;
    lis2mdl_data_t magnetometer;
    ULONG          timestamp_ticks;
} sensor_data_t;

/* Shared resources — owned by sensor_monitor, consumed by others */
extern TX_QUEUE  sensor_data_queue;
extern TX_EVENT_FLAGS_GROUP alert_flags;
extern TX_MUTEX  i2c_mutex;

/* Thread entry point */
void sensor_monitor_thread_entry(ULONG parameter);

/* Initialise shared kernel objects (queue, event flags, mutex).
   Must be called from tx_application_define(). */
UINT sensor_monitor_init(TX_BYTE_POOL *byte_pool);

#endif /* _SENSOR_MONITOR_H */
