/* 
 * Copyright (c) Microsoft
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     Microsoft         - Initial version
 *     Frédéric Desbiens - 2024 version.
 *     <Your Name>       - Multi-threaded Environmental Monitoring demo.
 */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "cmsis_utils.h"
#include "screen.h"
#include "sntp_client.h"
#include "wwd_networking.h"

#include "cloud_config.h"
#include "sensor_monitor.h"
#include "display_manager.h"
#include "alert_manager.h"

/* --------------------------------------------------------------------
   Thread configuration
   Priority assignments (lower number = higher priority):
     4 — Alert manager   (time-critical LED blinking)
     5 — Sensor monitor  (periodic I2C reads)
     6 — Display manager (UI rendering, lower priority than sensors)
     7 — Console logger  (background serial output)
     8 — Network init    (one-shot, lowest priority)
   -------------------------------------------------------------------- */
#define SENSOR_THREAD_STACK_SIZE   4096
#define SENSOR_THREAD_PRIORITY     5

#define DISPLAY_THREAD_STACK_SIZE  4096
#define DISPLAY_THREAD_PRIORITY    6

#define ALERT_THREAD_STACK_SIZE    2048
#define ALERT_THREAD_PRIORITY      4

#define CONSOLE_THREAD_STACK_SIZE  4096
#define CONSOLE_THREAD_PRIORITY    7

#define NETWORK_THREAD_STACK_SIZE  4096
#define NETWORK_THREAD_PRIORITY    8

/* Thread control blocks */
static TX_THREAD sensor_thread;
static TX_THREAD display_thread;
static TX_THREAD alert_thread;
static TX_THREAD console_thread;
static TX_THREAD network_thread;

/* Thread stacks */
static ULONG sensor_thread_stack[SENSOR_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG display_thread_stack[DISPLAY_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG alert_thread_stack[ALERT_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG console_thread_stack[CONSOLE_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG network_thread_stack[NETWORK_THREAD_STACK_SIZE / sizeof(ULONG)];

/* Byte pool for dynamic allocations (used by sensor_monitor_init) */
#define BYTE_POOL_SIZE  4096
static TX_BYTE_POOL byte_pool;
static ULONG byte_pool_storage[BYTE_POOL_SIZE / sizeof(ULONG)];

/* --------------------------------------------------------------------
   Console logger thread — prints latest sensor data to UART
   -------------------------------------------------------------------- */
static void console_thread_entry(ULONG parameter)
{
    (void)parameter;

    sensor_data_t snapshot;

    printf("Console logger: started\r\n");
    printf("-------------------------------------------\r\n");
    printf(" Eclipse ThreadX — Environmental Monitor\r\n");
    printf(" Sensor interval: %lu ticks\r\n", (unsigned long)SENSOR_READ_INTERVAL_TICKS);
    printf("-------------------------------------------\r\n\r\n");

    for (;;)
    {
        /* Wait for a fresh reading via the queue.  The display thread
           also reads from the same queue, but sensor_monitor posts
           at a fixed rate so both consumers get data.  If the queue
           is empty we sleep and retry — this thread is lowest priority. */
        UINT status = tx_queue_receive(&sensor_data_queue, &snapshot, 100);
        if (status != TX_SUCCESS)
        {
            /* No fresh data — sleep and retry */
            tx_thread_sleep(SENSOR_READ_INTERVAL_TICKS);
            continue;
        }

        printf("[t=%lu] Temp=%.1fC  Hum=%.1f%%  Press=%.1f hPa  "
               "Accel=(%.0f,%.0f,%.0f)mg  Mag=(%.0f,%.0f,%.0f)mG\r\n",
               (unsigned long)snapshot.timestamp_ticks,
               (double)snapshot.humidity.temperature_degC,
               (double)snapshot.humidity.humidity_perc,
               (double)snapshot.pressure.pressure_hPa,
               (double)snapshot.motion.acceleration_mg[0],
               (double)snapshot.motion.acceleration_mg[1],
               (double)snapshot.motion.acceleration_mg[2],
               (double)snapshot.magnetometer.magnetic_mG[0],
               (double)snapshot.magnetometer.magnetic_mG[1],
               (double)snapshot.magnetometer.magnetic_mG[2]);
    }
}

/* --------------------------------------------------------------------
   Network initialisation thread — runs once, then terminates
   -------------------------------------------------------------------- */
static void network_thread_entry(ULONG parameter)
{
    (void)parameter;
    UINT status;

    printf("Network init: starting WiFi...\r\n");

    if ((status = wwd_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\r\n", status);
        return;
    }

    wwd_network_connect();
    printf("Network init: complete\r\n");
}

/* --------------------------------------------------------------------
   ThreadX application entry — create all kernel objects and threads
   -------------------------------------------------------------------- */
void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;
    UINT status;

    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    /* Create the byte pool for shared allocations */
    status = tx_byte_pool_create(&byte_pool,
                                 "Byte Pool",
                                 byte_pool_storage,
                                 BYTE_POOL_SIZE);
    if (status != TX_SUCCESS)
    {
        printf("ERROR: Byte pool creation failed\r\n");
        return;
    }

    /* Initialise sensor monitor kernel objects (queue, mutex, flags, timer) */
    status = sensor_monitor_init(&byte_pool);
    if (status != TX_SUCCESS)
    {
        printf("ERROR: Sensor monitor init failed (0x%02x)\r\n", status);
        return;
    }

    /* --- Create threads --- */

    status = tx_thread_create(&sensor_thread,
        "Sensor Monitor",
        sensor_monitor_thread_entry,
        0,
        sensor_thread_stack,
        SENSOR_THREAD_STACK_SIZE,
        SENSOR_THREAD_PRIORITY,
        SENSOR_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: sensor thread\r\n"); return; }

    status = tx_thread_create(&display_thread,
        "Display Manager",
        display_manager_thread_entry,
        0,
        display_thread_stack,
        DISPLAY_THREAD_STACK_SIZE,
        DISPLAY_THREAD_PRIORITY,
        DISPLAY_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: display thread\r\n"); return; }

    status = tx_thread_create(&alert_thread,
        "Alert Manager",
        alert_manager_thread_entry,
        0,
        alert_thread_stack,
        ALERT_THREAD_STACK_SIZE,
        ALERT_THREAD_PRIORITY,
        ALERT_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: alert thread\r\n"); return; }

    status = tx_thread_create(&console_thread,
        "Console Logger",
        console_thread_entry,
        0,
        console_thread_stack,
        CONSOLE_THREAD_STACK_SIZE,
        CONSOLE_THREAD_PRIORITY,
        CONSOLE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: console thread\r\n"); return; }

    status = tx_thread_create(&network_thread,
        "Network Init",
        network_thread_entry,
        0,
        network_thread_stack,
        NETWORK_THREAD_STACK_SIZE,
        NETWORK_THREAD_PRIORITY,
        NETWORK_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: network thread\r\n"); return; }
}

int main(void)
{
    /* Initialize the board */
    board_init();

    /* Enter the ThreadX kernel */
    tx_kernel_enter();

    return 0;
}
