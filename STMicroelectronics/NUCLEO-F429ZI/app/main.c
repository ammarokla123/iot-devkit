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
 *     <Your Name> - NUCLEO-F429ZI Eclipse ThreadX starter application.
 */

/**
 * @file main.c
 * @brief Minimal ThreadX starter for the STM32 NUCLEO-F429ZI.
 *
 * Demonstrates core ThreadX primitives on real hardware:
 *   - Two threads communicating via a message queue
 *   - A periodic timer driving an LED heartbeat
 *   - Button input toggling an event flag
 *
 * Hardware used:
 *   LD1 (green, PB0)  — heartbeat blink (1 Hz)
 *   LD2 (blue, PB7)   — toggles when user button is pressed
 *   LD3 (red, PB14)   — blinks in the worker thread
 *   B1  (PC13)        — user button
 *   USART3            — serial console via ST-LINK VCP (115200 baud)
 */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"

/* ----- Thread configuration ----- */
#define HEARTBEAT_STACK_SIZE  2048
#define HEARTBEAT_PRIORITY    5

#define WORKER_STACK_SIZE     2048
#define WORKER_PRIORITY       6

/* ----- Kernel objects ----- */
static TX_THREAD heartbeat_thread;
static TX_THREAD worker_thread;

static ULONG heartbeat_stack[HEARTBEAT_STACK_SIZE / sizeof(ULONG)];
static ULONG worker_stack[WORKER_STACK_SIZE / sizeof(ULONG)];

static TX_TIMER  heartbeat_timer;
static TX_QUEUE  msg_queue;
static TX_EVENT_FLAGS_GROUP button_flags;

/* Queue storage — 8 messages of 1 ULONG each */
#define QUEUE_MSG_SIZE   1
#define QUEUE_DEPTH      8
static ULONG queue_storage[QUEUE_DEPTH];

/* Event flag bit for button press */
#define BUTTON_PRESS_FLAG  ((ULONG)0x01)

/* ----- SysTick helper (same pattern as MXChip) ----- */
static __inline void systick_interval_set(uint32_t ticks)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD = SystemCoreClock / ticks - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

/* ----- Timer callback: heartbeat LED blink at 1 Hz ----- */
static void heartbeat_timer_cb(ULONG id)
{
    (void)id;
    LED1_TOGGLE();
}

/* ----- Heartbeat thread: monitors button, sends messages ----- */
static void heartbeat_thread_entry(ULONG parameter)
{
    (void)parameter;
    ULONG counter = 0;
    int   btn_was_pressed = 0;

    printf("Heartbeat thread: started\r\n");

    /* Start the 1 Hz heartbeat timer */
    tx_timer_activate(&heartbeat_timer);

    for (;;)
    {
        /* Poll the user button with debounce */
        if (USER_BUTTON_IS_PRESSED)
        {
            if (!btn_was_pressed)
            {
                btn_was_pressed = 1;

                /* Signal the worker thread via event flag */
                tx_event_flags_set(&button_flags, BUTTON_PRESS_FLAG, TX_OR);

                /* Send a counter message to the worker */
                counter++;
                tx_queue_send(&msg_queue, &counter, TX_NO_WAIT);

                printf("Button pressed (#%lu)\r\n", counter);
                LED2_TOGGLE();
            }
        }
        else
        {
            btn_was_pressed = 0;
        }

        /* Poll every 50 ms */
        tx_thread_sleep(5);
    }
}

/* ----- Worker thread: receives messages, blinks LED3 ----- */
static void worker_thread_entry(ULONG parameter)
{
    (void)parameter;
    ULONG received;

    printf("Worker thread: started — waiting for button presses\r\n");

    for (;;)
    {
        /* Wait for a message from the heartbeat thread (up to 5 seconds) */
        UINT status = tx_queue_receive(&msg_queue, &received, 500);
        if (status == TX_SUCCESS)
        {
            printf("Worker: received message #%lu — blinking LED3\r\n", received);

            /* Blink the red LED a few times to acknowledge */
            for (int i = 0; i < 6; i++)
            {
                LED3_TOGGLE();
                tx_thread_sleep(10);
            }
        }
    }
}

/* ----- ThreadX application entry ----- */
void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;
    UINT status;

    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    /* Message queue */
    status = tx_queue_create(&msg_queue, "Msg Queue",
                             QUEUE_MSG_SIZE,
                             queue_storage, sizeof(queue_storage));
    if (status != TX_SUCCESS) { printf("ERROR: queue\r\n"); return; }

    /* Event flags */
    status = tx_event_flags_create(&button_flags, "Button Flags");
    if (status != TX_SUCCESS) { printf("ERROR: flags\r\n"); return; }

    /* Heartbeat timer — 50 ticks = 500 ms (blink at 1 Hz) */
    status = tx_timer_create(&heartbeat_timer, "Heartbeat",
                             heartbeat_timer_cb, 0,
                             50, 50, TX_NO_ACTIVATE);
    if (status != TX_SUCCESS) { printf("ERROR: timer\r\n"); return; }

    /* Heartbeat thread */
    status = tx_thread_create(&heartbeat_thread,
        "Heartbeat",
        heartbeat_thread_entry, 0,
        heartbeat_stack, HEARTBEAT_STACK_SIZE,
        HEARTBEAT_PRIORITY, HEARTBEAT_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: heartbeat thread\r\n"); return; }

    /* Worker thread */
    status = tx_thread_create(&worker_thread,
        "Worker",
        worker_thread_entry, 0,
        worker_stack, WORKER_STACK_SIZE,
        WORKER_PRIORITY, WORKER_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) { printf("ERROR: worker thread\r\n"); return; }
}

int main(void)
{
    board_init();
    tx_kernel_enter();
    return 0;
}
