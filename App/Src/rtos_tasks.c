/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "includes.h"

QueueHandle_t g_beep_queue;

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

    beep_data_t init_beep = { .times = 1, .on_period = 100, .off_period = 0 };

    taskENTER_CRITICAL();
    g_beep_queue = xQueueCreate(1, sizeof(beep_data_t));
    xTaskCreate(key_task, "key_task", 128, NULL, 3, &key_task_handle);
    xTaskCreate(adc_task, "adc_task", 128, NULL, 2, &adc_task_handle);
    xTaskCreate(beep_task, "beep_task", 128, NULL, 1, &beep_task_handle);
    taskEXIT_CRITICAL();

    xQueueOverwrite(g_beep_queue, &init_beep);

    PUMP_ON();
    LED_ON();

    vTaskDelete(start_task_handle);
    vTaskDelay(portMAX_DELAY);
}

/**
 * @brief Buzzer task. Waits on the beep semaphore and produces a short beep
 *        (100 ms) each time the semaphore is given.
 *
 * @param args Task arguments (unused).
 */
__NO_RETURN static void beep_task(void *args)
{
    beep_data_t beep_data;
    while (1) {
        if (xQueueReceive(g_beep_queue, &beep_data, portMAX_DELAY) != pdPASS) {
            continue;
        }

        for (size_t i = 0; i < beep_data.times; i++) {
            beep_on();
            vTaskDelay(pdMS_TO_TICKS(beep_data.on_period));
            beep_off();
            vTaskDelay(pdMS_TO_TICKS(beep_data.off_period));
        }
    }
}
