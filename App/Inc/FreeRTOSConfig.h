/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT-License
 */

#ifndef __FREERTOS_CONFIG_H
#define __FREERTOS_CONFIG_H

extern uint32_t SystemCoreClock;

/* Define CPU frequency, unit: Hz, no default, must be defined */
#define configCPU_CLOCK_HZ                        (SystemCoreClock)
/* Define SysTick clock frequency. Only define when SysTick clock frequency differs from kernel clock frequency.
 * Unit: Hz. Default: not defined */
// #define configSYSTICK_CLOCK_HZ                    (configCPU_CLOCK_HZ / 8)

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>Basic Configuration Items
// =============================================================================

//  <q>Use preemptive scheduling
//  <i> If preemptive scheduling is not used, cooperative scheduling will be used. Preemptive scheduling is recommended
#define configUSE_PREEMPTION                      1

//  <q>Use hardware to calculate the next task to run
//  <i> Default: 0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION   1

//  <q>Enable tickless low power mode
//  <i> If tickless mode is enabled, stop tick periodic interrupts when in Idle.
//  <i> If disabled, tick periodic interrupts will be generated continuously
//  <i> Default: 0
#define configUSE_TICKLESS_IDLE                   0

//  <o>System clock tick frequency [Hz] <0-0xFFFFFFFF>
#define configTICK_RATE_HZ                        ((TickType_t)1000)

//  <o>Maximum priority
#define configMAX_PRIORITIES                      32

//  <o>Idle task stack size [words] <0-65535>
#define configMINIMAL_STACK_SIZE                  ((uint16_t)(128))

//  <o>Maximum task name length [bytes]
//  <i> Default: 16
#define configMAX_TASK_NAME_LEN                   16

//  <q>Define system tick counter data type as uint16
#define configUSE_16_BIT_TICKS                    0

//  <q>Idle can be preempted
//  <i> In preemptive scheduling, tasks with the same priority can preempt the Idle task.
//  <i> Default: 1
#define configIDLE_SHOULD_YIELD                   1

//  <q>Enable direct message passing between tasks
//  <i> If disabled, semaphores, event flag groups, message mailboxes cannot be used
//  <i> Default: 1
#define configUSE_TASK_NOTIFICATIONS              1

//  <o>Define the size of the task notification array
//  <i> Default: 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES     1

//  <q>Enable mutex semaphores
//  <i> Default: 0
#define configUSE_MUTEXES                         1

//  <q>Enable recursive mutex semaphores
//  <i> Default: 0
#define configUSE_RECURSIVE_MUTEXES               1

//  <q>Enable counting semaphores
//  <i> Default: 0
#define configUSE_COUNTING_SEMAPHORES             1

//  <o>Number of semaphores and message queues that can be registered
//  <i> Default: 0
#define configQUEUE_REGISTRY_SIZE                 8

//  <q>Enable queue sets
//  <i> Default: 0
#define configUSE_QUEUE_SETS                      1

//  <q>Enable time slicing
//  <i> Default: 1
#define configUSE_TIME_SLICING                    1

//  <q>Allocate Newlib reentrant structure when creating tasks
//  <i> Default: 0
#define configUSE_NEWLIB_REENTRANT                0

//  <q>Backward compatibility
//  <i> Default: 1
#define configENABLE_BACKWARD_COMPATIBILITY       0

//  <o>Define the number of thread local storage pointers
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS   0

//  <o>Enable FPU
#define configENABLE_FPU                          1

// </h>

/* Define the data type for task stack depth, default: uint16_t */
#define configSTACK_DEPTH_TYPE                    uint16_t
/* Define the data type for message length in message buffers, default: size_t */
#define configMESSAGE_BUFFER_LENGTH_TYPE          size_t

// <h>Memory Allocation
// =============================================================================

//  <q>Support static memory allocation
//  <i> Default: 0
#define configSUPPORT_STATIC_ALLOCATION           0

//  <q>Support dynamic memory allocation
//  <i> Default: 1
#define configSUPPORT_DYNAMIC_ALLOCATION          1

//  <o>Total heap memory size [byte] <0-65535>
#define configTOTAL_HEAP_SIZE                     ((size_t)(10 * 1024) )

//  <q>User manually allocates FreeRTOS memory heap
//  <i> Default: 0
#define configAPPLICATION_ALLOCATED_HEAP          0

//  <q>User implements memory allocation and deallocation functions used when creating tasks
//  <i> Default: 0
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP 0

// </h>

// <h>Hook Functions Related
// =============================================================================

//  <q>Use Idle hook function
//  <i> Call vApplicationIdleHook hook function when idle
#define configUSE_IDLE_HOOK                       0

//  <q>Use Tick hook function
//  <i> Call vApplicationTickHook hook function when each tick interrupt occurs
#define configUSE_TICK_HOOK                       0

//  <o>Check for stack overflow
//    <0=>Disable <1=>Method 1 <2=>Method 2 <3=>Method 3
//  <i> When stack overflow occurs, call vApplicationStackOverflowHook hook function
//  <i> Refer to FreeRTOS official documentation for differences between methods
//  <i> Default: 0
#define configCHECK_FOR_STACK_OVERFLOW            0

//  <q>Enable Timer Service Startup hook function
//  <i> Call vApplicationDaemonTaskStartupHook hook function before timer service executes for the first time
//  <i> Default: 0
#define configUSE_DAEMON_TASK_STARTUP_HOOK        0

//  <q>Use dynamic memory allocation failure hook function
//  <i> Call vApplicationMallocFailedHook hook function when dynamic memory allocation fails
//  <i> Default: 0
#define configUSE_MALLOC_FAILED_HOOK              0

// </h>

// <h>Runtime and Task Status Statistics Related Definitions
// =============================================================================

//  <q>Enable task runtime statistics function
//  <i> Default: 0
#define configGENERATE_RUN_TIME_STATS             0

#if (configGENERATE_RUN_TIME_STATS == 1)
#include "btim.h"
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() ConfigureTimeForRunTimeStats()
extern uint32_t FreeRTOSRunTimeTicks;
#define portGET_RUN_TIME_COUNTER_VALUE() FreeRTOSRunTimeTicks
#endif /* configGENERATE_RUN_TIME_STATS == 1 */

//  <q>Use visual trace debugging
//  <i> Default: 0
#define configUSE_TRACE_FACILITY             1

//  <q>Use formatted output functions
//  <i> When enabled, vTaskList() and vTaskGetRunTimeStats() functions will be compiled.
//  <i> Default: 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

// </h>

// <h>Coroutine Related Definitions

//  <q>Enable coroutines
//  <i> Default: 0
#define configUSE_CO_ROUTINES                0

//  <o>Maximum coroutine priority
//  <i> Must be defined when coroutines are enabled
#define configMAX_CO_ROUTINE_PRIORITIES      2

// </h>

// <e>Enable Software Timers
// =============================================================================
#define configUSE_TIMERS                     1

//  <o>Software timer task stack size [words] <0-65535>
#define configTIMER_TASK_STACK_DEPTH         256

//  <o>Software timer task priority <0-56>
#define configTIMER_TASK_PRIORITY            31

//  <o>Software timer command queue length <0-1024>
#define configTIMER_QUEUE_LENGTH             5

// </e>

//------------- <<< end of configuration section >>> ---------------------------

/* Optional functions, 1: Enable */
#define INCLUDE_vTaskPrioritySet             1 /* Set task priority */
#define INCLUDE_uxTaskPriorityGet            1 /* Get task priority */
#define INCLUDE_vTaskDelete                  1 /* Delete task */
#define INCLUDE_vTaskSuspend                 1 /* Suspend task */
#define INCLUDE_xResumeFromISR               1 /* Resume task suspended in interrupt */
#define INCLUDE_vTaskDelayUntil              1 /* Task absolute delay */
#define INCLUDE_vTaskDelay                   1 /* Task delay */
#define INCLUDE_xTaskGetSchedulerState       1 /* Get task scheduler state */
#define INCLUDE_xTaskGetCurrentTaskHandle    1 /* Get current task handle */
#define INCLUDE_uxTaskGetStackHighWaterMark  1 /* Get task stack historical minimum remaining */
#define INCLUDE_xTaskGetIdleTaskHandle       1 /* Get idle task handle */
#define INCLUDE_eTaskGetState                1 /* Get task state */
#define INCLUDE_xEventGroupSetBitFromISR     1 /* Set event flag bit in interrupt */
#define INCLUDE_xTimerPendFunctionCall       1 /* Pend function execution to timer service task */
#define INCLUDE_xTaskAbortDelay              1 /* Abort task delay */
#define INCLUDE_xTaskGetHandle               1 /* Get task handle by name */
#define INCLUDE_xTaskResumeFromISR           1 /* Resume task suspended in interrupt */

// Interrupt nesting behavior configuration
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15 /* Lowest interrupt priority */
/* Highest interrupt priority that FreeRTOS can manage */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY                                        \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY                                   \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_API_CALL_INTERRUPT_PRIORITY                                  \
    configMAX_SYSCALL_INTERRUPT_PRIORITY

/* Assertion */
#if 0
#define configASSERT(x)                                                        \
    if ((x) == 0)                                                              \
    vAssertCalled(__FILE__, __LINE__)
#endif

#ifdef configASSERT
extern void vAssertCalled(const char *pcFile, unsigned int ulLine);
#endif /* configASSERT */

/* Redirect FreeRTOS interrupt service related functions to system interrupts */
#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler    SVC_Handler

#endif /* __FREERTOS_CONFIG_H */
