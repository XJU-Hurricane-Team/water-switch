/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "includes.h"

SemaphoreHandle_t beep_sem;
static TaskHandle_t beep_task_handle;
static void beep_task(void *args);

static void start_task(void *args);
static TaskHandle_t start_task_handle;

/**
 * @brief FreeRTOS start up.
 *
 */
void rtos_start(void)
{
    xTaskCreate(start_task, "start_task", 128, NULL, 1, &start_task_handle);
    vTaskStartScheduler();
}

/**
 * @brief Start task.
 *
 * @param args Parameters.
 */
static void start_task(void *args)
{
    UNUSED(args);

    taskENTER_CRITICAL();
    beep_sem = xSemaphoreCreateBinary();
    xTaskCreate(key_task, "key_task", 128, NULL, 3, &key_task_handle);
    xTaskCreate(adc_task, "adc_task", 128, NULL, 2, &adc_task_handle);
    xTaskCreate(beep_task, "beep_task", 128, NULL, 1, &beep_task_handle);
    taskEXIT_CRITICAL();

    xSemaphoreGive(beep_sem);
    PUMP_ON();
    LED_ON();

    vTaskDelete(start_task_handle);
    vTaskDelay(portMAX_DELAY);
}

__NO_RETURN static void beep_task(void *args)
{
    while (1) {
        xSemaphoreTake(beep_sem, portMAX_DELAY);
        beep_on();
        vTaskDelay(pdMS_TO_TICKS(100));
        beep_off();
    }
}
