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
/** auto on/off that water is more than upper limit, 
 *  and off/on when it less than lower limit */
#define PUMPING_MODE 2

// #define MODE_CONF    ALERT_MODE
#define MODE_CONF    PUMPING_MODE

extern TaskHandle_t adc_task_handle;
extern void adc_task(void *args);

#define SENSOR_CONNECT_ERROR_THRESHOLD (40)
#define SENSOR_UP_RESISTOR_KR          (10)
#define SENSOR_DOWN_RESISTOR_KR        (300)

#define WATER_MAX_LEVEL                (4095)
/* sensor disconnect limit. */
#define DISCONNECT_LIMIT               (SENSOR_CONNECT_ERROR_THRESHOLD + (WATER_MAX_LEVEL * SENSOR_UP_RESISTOR_KR) / (SENSOR_UP_RESISTOR_KR + SENSOR_DOWN_RESISTOR_KR))
#define WATER_MIN_LEVEL                (DISCONNECT_LIMIT + 30)
#define WATER_INIT_UPPER_LEVEL         (3800)
#define WATER_INIT_LOWER_LEVEL         (300)
#define ON_MAX_TIME_INIT_SEC           (330)

typedef struct __packed {
    uint16_t upper_limit; /* upper limit */
#if MODE_CONF == PUMPING_MODE
    uint16_t lower_limit;     /* lower limit */
#endif                        /* MODE_CONF == PUMPING_MODE */
    uint16_t on_max_time_sec; /* turn on max time, unit: second */
    uint8_t crc8;             /* CRC8 value */
} limit_data_t;

extern limit_data_t g_limit_data;

extern TaskHandle_t key_task_handle;
extern void key_task(void *args);

void adc_save_limit(void);

/**
 * @brief beep data, for beep task.
 */
typedef struct {
    uint8_t times;       /** beep times */
    uint32_t on_period;  /** beep on period, unit: ms */
    uint32_t off_period; /** beep off period, unit: ms */
} beep_data_t;

extern QueueHandle_t g_beep_queue;
