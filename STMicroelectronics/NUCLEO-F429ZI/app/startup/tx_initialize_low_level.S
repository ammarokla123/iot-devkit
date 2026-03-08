@/**************************************************************************/
@/*                                                                        */
@/*       This program and the accompanying materials are made available   */
@/*       under the terms of the MIT license which is available at         */
@/*       https://opensource.org/license/mit                               */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/*       SPDX-License-Identifier: MIT                                     */
@/*                                                                        */
@/**************************************************************************/
@
@/* ThreadX low-level initialisation for NUCLEO-F429ZI (Cortex-M4 @ 180 MHz) */
@
@#define TX_SOURCE_CODE
@
@#include "tx_api.h"
@#include "tx_initialize.h"
@#include "tx_thread.h"
@#include "tx_timer.h"
@
@
    .global     _tx_thread_system_stack_ptr
    .global     _tx_initialize_unused_memory
    .global     __RAM_segment_used_end__
    .global     _tx_timer_interrupt
    .global     __main
    .global     __tx_SVCallHandler
    .global     __tx_PendSVHandler
    .global     _vectors
    .global     __tx_NMIHandler
    .global     __tx_BadHandler
    .global     __tx_SVCallHandler
    .global     __tx_DBGHandler
    .global     __tx_PendSVHandler
    .global     __tx_SysTickHandler
    .global     __tx_IntHandler
@
@
@ NUCLEO-F429ZI: 180 MHz system clock, 100 Hz tick rate
SYSTEM_CLOCK      =   180000000
SYSTICK_CYCLES    =   ((SYSTEM_CLOCK / 100) -1)

    .text 32
    .align 4
    .syntax unified

@VOID   _tx_initialize_low_level(VOID)
@{
    .global  _tx_initialize_low_level
    .thumb_func
_tx_initialize_low_level:
@
@    /* Disable interrupts during ThreadX initialization.  */
@
    CPSID   i
@
@    /* Set base of available memory to end of non-initialised RAM area.  */
@
    LDR     r0, =_tx_initialize_unused_memory
    LDR     r1, =__RAM_segment_used_end__
    ADD     r1, r1, #4
    STR     r1, [r0]
@
@    /* Setup Vector Table Offset Register.  */
@
    MOV     r0, #0xE000E000
    LDR     r1, =_vectors
    STR     r1, [r0, #0xD08]
@
@    /* Set system stack pointer from vector value.  */
@
    LDR     r0, =_tx_thread_system_stack_ptr
    LDR     r1, =_vectors
    LDR     r1, [r1]
    STR     r1, [r0]
@
@    /* Enable the cycle count register.  */
@
    LDR     r0, =0xE0001000
    LDR     r1, [r0]
    ORR     r1, r1, #1
    STR     r1, [r0]
@
@    /* Configure SysTick for 100Hz clock.  */
@
    MOV     r0, #0xE000E000
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]           @ SysTick Reload Value
    MOV     r1, #0x7                   @ Enable SysTick, use processor clock, enable interrupt
    STR     r1, [r0, #0x10]           @ SysTick Control
@
@    /* Configure handler priorities.  */
@
    LDR     r1, =0x00000000
    STR     r1, [r0, #0xD18]          @ System Handlers 4-7

    LDR     r1, =0xFF000000           @ SVC must be lowest priority (0xFF)
    STR     r1, [r0, #0xD1C]          @ System Handlers 8-11

    LDR     r1, =0x40FF0000           @ PendSV = 0xFF, SysTick = 0x40
    STR     r1, [r0, #0xD20]          @ System Handlers 12-15
@
@    /* Return to caller.  */
@
    BX      lr
@}
@

    .global  __tx_BadHandler
    .thumb_func
__tx_BadHandler:
    B       __tx_BadHandler

    .global  __tx_HardfaultHandler
    .thumb_func
__tx_HardfaultHandler:
    B       __tx_HardfaultHandler

    .global  __tx_SVCallHandler
    .thumb_func
__tx_SVCallHandler:
    B       __tx_SVCallHandler

    .global  __tx_IntHandler
    .thumb_func
__tx_IntHandler:
    PUSH    {lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter
#endif
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit
#endif
    POP     {lr}
    BX      lr

    .global  __tx_SysTickHandler
    .global  SysTick_Handler
    .thumb_func
__tx_SysTickHandler:
SysTick_Handler:
    PUSH    {lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter
#endif
    BL      _tx_timer_interrupt
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit
#endif
    POP     {lr}
    BX      lr

    .global  __tx_NMIHandler
    .thumb_func
__tx_NMIHandler:
    B       __tx_NMIHandler

    .global  __tx_DBGHandler
    .thumb_func
__tx_DBGHandler:
    B       __tx_DBGHandler

    .end
