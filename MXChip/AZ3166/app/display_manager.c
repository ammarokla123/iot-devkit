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

#include "display_manager.h"
#include "sensor_monitor.h"
#include "board_init.h"

#include <stdio.h>
#include <string.h>

#include "ssd1306.h"
#include "ssd1306_fonts.h"

/* Debounce time for buttons in ticks (100 ms at 100 ticks/s) */
#define BUTTON_DEBOUNCE_TICKS  10

/* Display refresh period — poll queue every 200 ms */
#define DISPLAY_POLL_TICKS     20

/* We cache the latest reading so the display always has data */
static sensor_data_t latest;
static int           has_data = 0;

/* Current page index */
static display_page_t current_page = DISPLAY_PAGE_CLIMATE;

/* ------------------------------------------------------------------
   Rendering helpers — each page fills the 128×64 OLED
   Font_11x18: ~11 chars per line, 4 lines (y = 0, 18, 36, 54)
   ------------------------------------------------------------------ */

static void render_climate(const sensor_data_t *d)
{
    char line[16];

    ssd1306_Fill(Black);

    ssd1306_SetCursor(2, 0);
    ssd1306_WriteString("Climate", Font_11x18, White);

    snprintf(line, sizeof(line), "T: %.1fC", (double)d->humidity.temperature_degC);
    ssd1306_SetCursor(2, 18);
    ssd1306_WriteString(line, Font_11x18, White);

    snprintf(line, sizeof(line), "H: %.1f%%", (double)d->humidity.humidity_perc);
    ssd1306_SetCursor(2, 36);
    ssd1306_WriteString(line, Font_11x18, White);

    ssd1306_SetCursor(2, 54);
    ssd1306_WriteString("[A]< [B]>", Font_11x18, White);

    ssd1306_UpdateScreen();
}

static void render_pressure(const sensor_data_t *d)
{
    char line[16];

    ssd1306_Fill(Black);

    ssd1306_SetCursor(2, 0);
    ssd1306_WriteString("Pressure", Font_11x18, White);

    snprintf(line, sizeof(line), "%.1f hPa", (double)d->pressure.pressure_hPa);
    ssd1306_SetCursor(2, 18);
    ssd1306_WriteString(line, Font_11x18, White);

    snprintf(line, sizeof(line), "T: %.1fC", (double)d->pressure.temperature_degC);
    ssd1306_SetCursor(2, 36);
    ssd1306_WriteString(line, Font_11x18, White);

    ssd1306_SetCursor(2, 54);
    ssd1306_WriteString("[A]< [B]>", Font_11x18, White);

    ssd1306_UpdateScreen();
}

static void render_motion(const sensor_data_t *d)
{
    char line[16];

    ssd1306_Fill(Black);

    ssd1306_SetCursor(2, 0);
    ssd1306_WriteString("Motion", Font_11x18, White);

    snprintf(line, sizeof(line), "X:%.0f", (double)d->motion.acceleration_mg[0]);
    ssd1306_SetCursor(2, 18);
    ssd1306_WriteString(line, Font_11x18, White);

    snprintf(line, sizeof(line), "Y:%.0f Z:%.0f",
             (double)d->motion.acceleration_mg[1],
             (double)d->motion.acceleration_mg[2]);
    ssd1306_SetCursor(2, 36);
    ssd1306_WriteString(line, Font_11x18, White);

    ssd1306_SetCursor(2, 54);
    ssd1306_WriteString("[A]< [B]>", Font_11x18, White);

    ssd1306_UpdateScreen();
}

static void render_compass(const sensor_data_t *d)
{
    char line[16];

    ssd1306_Fill(Black);

    ssd1306_SetCursor(2, 0);
    ssd1306_WriteString("Compass", Font_11x18, White);

    snprintf(line, sizeof(line), "X:%.0f", (double)d->magnetometer.magnetic_mG[0]);
    ssd1306_SetCursor(2, 18);
    ssd1306_WriteString(line, Font_11x18, White);

    snprintf(line, sizeof(line), "Y:%.0f", (double)d->magnetometer.magnetic_mG[1]);
    ssd1306_SetCursor(2, 36);
    ssd1306_WriteString(line, Font_11x18, White);

    snprintf(line, sizeof(line), "Z:%.0f", (double)d->magnetometer.magnetic_mG[2]);
    ssd1306_SetCursor(2, 54);
    ssd1306_WriteString(line, Font_11x18, White);

    ssd1306_UpdateScreen();
}

/* Render the appropriate page */
static void render_current_page(void)
{
    if (!has_data)
    {
        ssd1306_Fill(Black);
        ssd1306_SetCursor(2, 18);
        ssd1306_WriteString("Waiting...", Font_11x18, White);
        ssd1306_UpdateScreen();
        return;
    }

    switch (current_page)
    {
        case DISPLAY_PAGE_CLIMATE:  render_climate(&latest);  break;
        case DISPLAY_PAGE_PRESSURE: render_pressure(&latest); break;
        case DISPLAY_PAGE_MOTION:   render_motion(&latest);   break;
        case DISPLAY_PAGE_COMPASS:  render_compass(&latest);  break;
        default:                    render_climate(&latest);  break;
    }
}

/* ------------------------------------------------------------------
   Thread entry
   ------------------------------------------------------------------ */
void display_manager_thread_entry(ULONG parameter)
{
    (void)parameter;

    ULONG  btn_a_last_release = 0;
    ULONG  btn_b_last_release = 0;
    int    needs_redraw = 1;

    printf("Display manager: started\r\n");

    /* Show splash on boot */
    ssd1306_Fill(Black);
    ssd1306_SetCursor(2, 0);
    ssd1306_WriteString("Eclipse", Font_11x18, White);
    ssd1306_SetCursor(2, 18);
    ssd1306_WriteString("ThreadX", Font_11x18, White);
    ssd1306_SetCursor(2, 36);
    ssd1306_WriteString("EnvMonitor", Font_11x18, White);
    ssd1306_UpdateScreen();

    /* Let the splash screen stay for 2 seconds */
    tx_thread_sleep(200);

    for (;;)
    {
        /* Drain the sensor queue — always grab the freshest reading */
        sensor_data_t incoming;
        while (tx_queue_receive(&sensor_data_queue, &incoming, TX_NO_WAIT) == TX_SUCCESS)
        {
            latest   = incoming;
            has_data = 1;
            needs_redraw = 1;
        }

        /* Button A — previous page */
        ULONG now = tx_time_get();
        if (BUTTON_A_IS_PRESSED && (now - btn_a_last_release) > BUTTON_DEBOUNCE_TICKS)
        {
            current_page = (current_page == 0)
                ? (DISPLAY_PAGE_COUNT - 1)
                : (current_page - 1);
            needs_redraw = 1;
            btn_a_last_release = now;
        }

        /* Button B — next page */
        if (BUTTON_B_IS_PRESSED && (now - btn_b_last_release) > BUTTON_DEBOUNCE_TICKS)
        {
            current_page = (current_page + 1) % DISPLAY_PAGE_COUNT;
            needs_redraw = 1;
            btn_b_last_release = now;
        }

        /* Redraw only when data or page changed */
        if (needs_redraw)
        {
            /* Acquire I2C bus — display shares I2C1 with the sensors */
            tx_mutex_get(&i2c_mutex, TX_WAIT_FOREVER);
            render_current_page();
            tx_mutex_put(&i2c_mutex);
            needs_redraw = 0;
        }

        tx_thread_sleep(DISPLAY_POLL_TICKS);
    }
}
