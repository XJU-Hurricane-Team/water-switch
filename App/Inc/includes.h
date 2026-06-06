/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "main.h"

#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

#include <hc595/hc595.h>

/** alert and stop pump when water is more than limit */
#define ALERT_MODE   1
/** auto start that water is more than upper limit, 
 *  and stop when it less than lower limit */
#define PUMPING_MODE 2

// #define MODE_CONF    ALERT_MODE
#define MODE_CONF    PUMPING_MODE

extern TaskHandle_t adc_task_handle;
extern bool g_stop_display_adc;
extern void adc_task(void *args);

extern uint16_t g_upper_limit;

#define SENSOR_CONNECT_ERROR_THRESHOLD (40)
#define SENSOR_UP_RESISTOR_KR          (10)
#define SENSOR_DOWN_RESISTOR_KR        (300)

#define WATER_MAX_LEVEL                (4095)
/* sensor disconnect limit. */
#define DISCONNECT_LIMIT               (SENSOR_CONNECT_ERROR_THRESHOLD + (WATER_MAX_LEVEL * SENSOR_UP_RESISTOR_KR) / (SENSOR_UP_RESISTOR_KR + SENSOR_DOWN_RESISTOR_KR))
#define WATER_MIN_LEVEL                (DISCONNECT_LIMIT + 30)

#define WATER_INIT_UPPER_LEVEL         (3800)
#if MODE_CONF == PUMPING_MODE
extern uint16_t g_lower_limit;
#define WATER_INIT_LOWER_LEVEL (300)
#endif /* MODE_CONF == PUMPING_MODE */

extern TaskHandle_t key_task_handle;
extern void key_task(void *args);

void adc_save_limit(void);

extern SemaphoreHandle_t beep_sem;
