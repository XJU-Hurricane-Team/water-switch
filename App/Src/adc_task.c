/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "includes.h"

#define LIMIT_DATA_ADDRESS         0x08008000
#define ADC_BUF_SIZE               200
#define ADC_CHANGE_THRESHOLD       100
#define DISPLAY_UPDATE_PERIOD_TICK 200

/**
 * @brief threshold condition
 */
typedef enum {
    THRESHOLD_LT, /** less than */
    THRESHOLD_GT  /** more than */
} threshold_type_t;

/**
 * @brief threshold determination 
 */
typedef struct {
    int16_t threshold;     /** threshold value */
    threshold_type_t type; /** threshold condition */
    uint32_t duration_ms;  /** duration, unit: ms */
    uint32_t start_tick;   /** first ticks to meet condition */
    bool triggered;        /** is triggered */
} threshold_t;

/* adc buffer */
static uint16_t adc_buf[ADC_BUF_SIZE];
/* current water level */
static uint16_t water_level;
/* adc convert complete semaphore */
static SemaphoreHandle_t adc_conv_cplt_sem;

static void display_update(void);
static bool threshold_update(threshold_t *t, uint16_t value);
static void adc_read_limit(void);
static uint16_t adc_get_water_level(void);
static void disconnect_alert(void);

uint16_t g_upper_limit; /* upper threshold */
#if MODE_CONF == PUMPING_MODE
uint16_t g_lower_limit; /* lower threshold */
#endif                  /* MODE_CONF == PUMPING_MODE */

TaskHandle_t adc_task_handle;

enum {
    THRESHOLD_IDX_DISCONNECT = 0 /** disconnect threshold */,
    THRESHOLD_IDX_UPPER, /** upper threshold */
#if MODE_CONF == PUMPING_MODE
    THRESHOLD_IDX_LOWER /** lower threshold */
#endif                  /* MODE_CONF == PUMPING_MODE */
};

static threshold_t threshold_table[] = {
    { .threshold = DISCONNECT_LIMIT, .type = THRESHOLD_LT, .duration_ms = 3000 }, /** disconnect threshold */
    { .type = THRESHOLD_GT, .duration_ms = 2500 },                                /** upper threshold */
#if MODE_CONF == PUMPING_MODE
    { .type = THRESHOLD_LT, .duration_ms = 2500 }, /** lower threshold */
#endif                                             /* MODE_CONF == PUMPING_MODE */
};

/**
 * @brief ADC task. Reads water level via DMA, evaluates thresholds, and
 *        controls pump/alert outputs accordingly.
 *
 * @param args Task arguments (unused).
 */
__NO_RETURN void adc_task(void *args)
{
    UNUSED(args);
    adc_read_limit();

    beep_data_t beep = { .times = 1, .on_period = 75, .off_period = 75 };

    threshold_table[THRESHOLD_IDX_UPPER]
        .threshold = g_upper_limit;
#if MODE_CONF == PUMPING_MODE
    threshold_table[THRESHOLD_IDX_LOWER].threshold = g_lower_limit;
#endif /* MODE_CONF == PUMPING_MODE */

    adc_conv_cplt_sem = xSemaphoreCreateBinary();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, ADC_BUF_SIZE);

    while (1) {
        water_level = adc_get_water_level();

        if (threshold_update(&threshold_table[THRESHOLD_IDX_DISCONNECT], water_level)) {
            /* reach the disconnect threshold */
            disconnect_alert();
        }

#if MODE_CONF == PUMPING_MODE
        if (threshold_update(&threshold_table[THRESHOLD_IDX_LOWER], water_level)) {
            /* reach lower threshold */
            pump_on();
            /* beep two times */
            beep.times = 2;
            xQueueOverwrite(g_beep_queue, &beep);
        } else if (threshold_update(&threshold_table[THRESHOLD_IDX_UPPER], water_level)) {
            /* reach upper threshold */
            pump_off();
            /* beep three times */
            beep.times = 3;
            xQueueOverwrite(g_beep_queue, &beep);
        }

#elif MODE_CONF == ALERT_MODE
        if (threshold_update(&threshold_table[THRESHOLD_IDX_UPPER], water_level)) {
            /* reach upper threshold, turn pump off */
            pump_off();
            /* let the buzzer sound continuously */
            beep_on();
        }
#endif /* MODE_CONF */

#if PUMP_ON_MAX_SECONDS
        if (pump_is_on() && (HAL_GetTick() - g_pump_on_tick >= (PUMP_ON_MAX_SECONDS * 1000))) {
            /* reach open max time, pump still on, maybe no water, turn it off */
            pump_off();
            /* beep four times */
            beep.times = 4;
            xQueueOverwrite(g_beep_queue, &beep);
        }
#endif /* PUMP_ON_MAX_SECONDS */

        display_update();
    }
}

/**
 * @brief ADC conversion complete callback. Gives the ADC semaphore from ISR
 *        to signal that a new buffer of samples is ready.
 *
 * @param hadc ADC handle that triggered the callback.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
    xSemaphoreGiveFromISR(adc_conv_cplt_sem, NULL);
}

/**
 * @brief CRC-8 over @p len bytes.
 *        Polynomial 0x07 (x⁸ + x² + x + 1), init 0x00, no final XOR.
 * @param data data
 * @param len len
 */
static uint8_t crc8_calc(const uint8_t *data, uint32_t len)
{
    uint8_t crc = 0x00;
    while (len--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

typedef struct __packed {
    uint16_t upper_limit; /* upper limit */
#if MODE_CONF == PUMPING_MODE
    uint16_t lower_limit; /* lower limit */
#endif                    /* MODE_CONF == PUMPING_MODE */
    uint8_t crc8;         /* CRC8 value */
} limit_data_t;

/**
 * @brief Read limit values from flash. If CRC validation fails, the limits
 *        are reset to their default values and saved back to flash.
 */
static void adc_read_limit(void)
{
    limit_data_t data;
    memcpy(&data, (const void *)LIMIT_DATA_ADDRESS, sizeof(data));

    if (crc8_calc((const uint8_t *)&data, sizeof(data))) {
        g_upper_limit = WATER_INIT_UPPER_LEVEL;
#if MODE_CONF == PUMPING_MODE
        g_lower_limit = WATER_INIT_LOWER_LEVEL;
#endif /* MODE_CONF == PUMPING_MODE */
        adc_save_limit();
        return;
    }

    g_upper_limit = data.upper_limit;
#if MODE_CONF == PUMPING_MODE
    g_lower_limit = data.lower_limit;
#endif /* MODE_CONF == PUMPING_MODE */
}

/**
 * @brief Save current limit values to flash with a CRC-8 checksum.
 */
void adc_save_limit(void)
{
    limit_data_t data __ALIGNED(8);
    FLASH_EraseInitTypeDef erase_init_struct;

    data.upper_limit = g_upper_limit;
#if MODE_CONF == PUMPING_MODE
    data.lower_limit = g_lower_limit;
#endif /* MODE_CONF == PUMPING_MODE */
    data.crc8 = crc8_calc((const uint8_t *)&data, offsetof(limit_data_t, crc8));

    uint32_t error_code;

    __disable_irq();
    HAL_FLASH_Unlock();
    erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init_struct.NbPages = 1;
    erase_init_struct.PageAddress = LIMIT_DATA_ADDRESS;
    HAL_FLASHEx_Erase(&erase_init_struct, &error_code);

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, LIMIT_DATA_ADDRESS,
                      *(uint64_t *)&data);
    HAL_FLASH_Lock();
    __enable_irq();
}

/**
 * @brief Wait for the ADC DMA buffer to be filled, then compute the average
 *        and convert it to a water level value.
 *
 * @return Current water level (inverted and scaled from raw ADC readings).
 */
static uint16_t adc_get_water_level(void)
{
    xSemaphoreTake(adc_conv_cplt_sem, portMAX_DELAY);
    uint32_t total = 0;
    static int32_t filtered = -1;
    for (size_t i = 0; i < ADC_BUF_SIZE; i++) {
        total += adc_buf[i];
    }

    /** Water level and ADC value are inversely proportional; 
      * WATER_MAX_LEVEL minus ADC value is more intuitive. */

    uint16_t average = (uint16_t)(total / ADC_BUF_SIZE);
    if (filtered < 0) {
        filtered = average;
    } else {
        filtered += (average - filtered) / 8;
    }

    return WATER_MAX_LEVEL - filtered;
}

/**
 * @brief Handle sensor disconnect. Suspends the key task, saves pump state,
 *        toggles the buzzer, and waits until the sensor reconnects.
 */
static void disconnect_alert(void)
{
    /* change to more than threshold */
    threshold_table[THRESHOLD_IDX_DISCONNECT].type = THRESHOLD_GT;
    vTaskSuspend(key_task_handle);
    uint32_t pump_last_status = pump_is_on();
    pump_off();

    TickType_t start_tick = xTaskGetTickCount();
    while (1) {
        water_level = adc_get_water_level();
        if (threshold_update(&threshold_table[THRESHOLD_IDX_DISCONNECT], water_level)) {
            beep_off();
            break;
        }
        if (xTaskGetTickCount() - start_tick > 100) {
            beep_toggle();
            start_tick = xTaskGetTickCount();
        }

        display_update();
    }

    if (pump_last_status) {
        pump_on();
    } else {
        pump_off();
    }

    /* back to less than threshold */
    threshold_table[THRESHOLD_IDX_DISCONNECT].type = THRESHOLD_LT;
    vTaskResume(key_task_handle);
}

/**
 * @brief Stateful threshold check with debounce. Returns true only after the
 *        value has been beyond the threshold continuously for the configured
 *        duration.
 *
 * @param t     Pointer to the threshold descriptor.
 * @param value Current measured value.
 * @retval true  Threshold condition met and held for the required duration.
 * @retval false Threshold condition not met or not yet held long enough.
 */
static bool threshold_update(threshold_t *t, uint16_t value)
{
    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
    /* Does it meet the conditions? */
    uint8_t cond = 0;

    if (t->type == THRESHOLD_LT) {
        cond = (value < t->threshold);
    } else {
        cond = (value > t->threshold);
    }

    if (cond) {
        if (t->start_tick == 0) {
            t->start_tick = now;
        }

        if ((t->triggered == false) && (now - t->start_tick >= t->duration_ms)) {
            t->triggered = true;
            return true;
        }
    } else {
        t->start_tick = 0;
        t->triggered = false;
    }
    return false;
}

/**
 * @brief Periodically refresh the 7-segment display with the current water
 *        level, at a fixed rate of DISPLAY_UPDATE_PERIOD_TICK ticks.
 */
static void display_update(void)
{
    static TickType_t tick_start;
    if (xTaskGetTickCount() - tick_start >= DISPLAY_UPDATE_PERIOD_TICK) {
        hc595_display_uint16(water_level);
        tick_start = xTaskGetTickCount();
    }
}