/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "includes.h"

#define KEY_BUTTON             (1U << 0)
#define KEY_ENCODER            (1U << 1)

#define PRESS_DURATION_MS      20
#define ADJUST_WAIT_TIME_MS    (30 * 1000)

#define ADJUST_RES_OK          0
#define ADJUST_RES_CANCELED    -1
#define ADJUST_RES_OPT_TIMEOUT -2

static EventGroupHandle_t key_event;
TaskHandle_t key_task_handle;

static int adjust_value(uint16_t current_num, const char *name, uint16_t min_val, uint16_t max_val);
static int adjust_limit(void);

/**
 * @brief Key / encoder task. Waits for button and encoder events and
 *        dispatches pump toggle or limit adjustment accordingly.
 *
 * @param args Task arguments (unused).
 */
__NO_RETURN void key_task(void *args)
{
    UNUSED(args);

    key_event = xEventGroupCreate();
    EventBits_t key_event_bits;

    BUTTON_IRQ_ENABLE();
    ENC_KEY_IRQ_ENABLE();

    while (1) {
        key_event_bits = xEventGroupWaitBits(key_event, KEY_BUTTON | KEY_ENCODER, pdTRUE, pdFALSE, portMAX_DELAY);
        if (key_event_bits & KEY_BUTTON) {
            pump_toggle();
        }
        if (key_event_bits & KEY_ENCODER) {
            vTaskSuspend(adc_task_handle);

            uint32_t last_pump_state = pump_is_on();
            pump_off();
            encoder_start();

            if (adjust_limit() == ADJUST_RES_OK) {
                adc_save_limit();
            }

            if (last_pump_state) {
                pump_on();
            }

            encoder_stop();
            vTaskResume(adc_task_handle);
        }
    }
}

/**
 * @brief Orchestrate the interactive limit-adjustment flow using the rotary
 *        encoder. Depending on MODE_CONF, the user can set upper limit, lower
 *        limit (PUMPING_MODE only), and max-on-time.  Each parameter is
 *        edited digit-by-digit via adjust_value().
 *
 * @retval ADJUST_RES_OK          (0)  All values confirmed.
 * @retval ADJUST_RES_CANCELED    (-1) User canceled during a step.
 * @retval ADJUST_RES_OPT_TIMEOUT (-2) No operation for 30 s during a step.
 */
static int adjust_limit(void)
{
    limit_data_t new_data = g_limit_data;
    int new_value;
#if MODE_CONF == PUMPING_MODE
    new_value = adjust_value(g_limit_data.upper_limit,
                             "-UP-",
                             WATER_MIN_LEVEL + 50,
                             WATER_MAX_LEVEL);
    if (new_value == ADJUST_RES_OPT_TIMEOUT) {
        return ADJUST_RES_OPT_TIMEOUT;
    } else if (new_value != ADJUST_RES_CANCELED) {
        new_data.upper_limit = (uint16_t)new_value;
    }

    new_value = adjust_value(g_limit_data.lower_limit,
                             "-dn-",
                             WATER_MIN_LEVEL,
                             new_data.upper_limit - 30); /* do not more than upper limit */
    if (new_value == ADJUST_RES_OPT_TIMEOUT) {
        return ADJUST_RES_OPT_TIMEOUT;
    } else if (new_value != ADJUST_RES_CANCELED) {
        new_data.lower_limit = (uint16_t)new_value;
    }

    new_value = adjust_value(g_limit_data.on_max_time_sec,
                             "-oN-",
                             0,
                             9999);
    if (new_value == ADJUST_RES_OPT_TIMEOUT) {
        return ADJUST_RES_OPT_TIMEOUT;
    } else if (new_value != ADJUST_RES_CANCELED) {
        new_data.on_max_time_sec = (uint16_t)new_value;
    }

#elif MODE_CONF == ALERT_MODE
    new_value = adjust_value(g_limit_data.upper_limit,
                             "-UP-",
                             WATER_MIN_LEVEL,
                             WATER_MAX_LEVEL);
    if (new_value == ADJUST_RES_OPT_TIMEOUT) {
        return ADJUST_RES_OPT_TIMEOUT;
    } else if (new_value != ADJUST_RES_CANCELED) {
        new_data.upper_limit = (uint16_t)new_value;
    }

    new_value = adjust_value(g_limit_data.on_max_time_sec,
                             "-oN-",
                             0,
                             9999);
    if (new_value == ADJUST_RES_OPT_TIMEOUT) {
        return ADJUST_RES_OPT_TIMEOUT;
    } else if (new_value != ADJUST_RES_CANCELED) {
        new_data.on_max_time_sec = (uint16_t)new_value;
    }
#endif /* MODE_CONF */
    g_limit_data = new_data;
    return ADJUST_RES_OK;
}

/**
 * @brief Digit-by-digit limit adjustment using the rotary encoder.
 *        The user cycles through digit positions, adjusts each digit, and
 *        confirms or cancels the new value.
 *
 * @param current_num Current limit value to start adjusting from.
 * @param name        Short name shown on the 7-segment display (e.g. "-UP-").
 * @param min_val     Minimum allowed value (clamped).
 * @param max_val     Maximum allowed value (clamped).
 * @return result
 *  @retval ADJUST_RES_CANCELED    (-1) Canceled by button press.
 *  @retval ADJUST_RES_OPT_TIMEOUT (-2) No operation for 30 s.
 *  @retval other                   New confirmed value (clamped to [min_val, max_val]).
 */
static int adjust_value(uint16_t current_num, const char *name, uint16_t min_val, uint16_t max_val)
{
    EventBits_t key_event_bits;

    /* display name first */
    hc595_display_str(name);
    key_event_bits = xEventGroupWaitBits(key_event, KEY_BUTTON | KEY_ENCODER, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
    if (key_event_bits & KEY_BUTTON) {
        /* user canceled */
        return ADJUST_RES_CANCELED;
    }

    /* don't move position in display name duration */
    xEventGroupClearBits(key_event, KEY_ENCODER);
    hc595_display_uint16(current_num);

    int pos = 3;
    char num[4];

    for (int i = 0; i < 4; i++) {
        num[i] = (current_num % 10) + '0';
        current_num /= 10;
    }

    char cur_display_num = ' ';
    char last_display_num;

    TickType_t now_tick = xTaskGetTickCount();
    TickType_t display_tick = now_tick;
    TickType_t edit_tick = now_tick;

    uint16_t enc_cnt = encoder_get_count();
    uint16_t last_enc_cnt = enc_cnt;

    xEventGroupClearBits(key_event, KEY_BUTTON | KEY_ENCODER);

    while (1) {
        key_event_bits = xEventGroupWaitBits(key_event, KEY_BUTTON | KEY_ENCODER, pdTRUE, pdFALSE, pdMS_TO_TICKS(200));
        if (key_event_bits & KEY_BUTTON) {
            return ADJUST_RES_CANCELED;
        }

        now_tick = xTaskGetTickCount();

        if (key_event_bits & KEY_ENCODER) {
            hc595_display_num_pos(num[pos], pos, false);
            pos--;
            if (pos < 0) {
                break;
            }
            edit_tick = now_tick;
        }

        last_display_num = num[pos];
        enc_cnt = encoder_get_count();
        if (last_enc_cnt < enc_cnt) {
            last_enc_cnt = enc_cnt;
            num[pos]++;
        } else if (last_enc_cnt > enc_cnt) {
            last_enc_cnt = enc_cnt;
            num[pos]--;
        }
        if (last_display_num != num[pos]) {
            if (num[pos] < '0') {
                num[pos] = '9';
            } else if (num[pos] > '9') {
                num[pos] = '0';
            }
            hc595_display_num_pos(num[pos], pos, false);
            last_display_num = num[pos];
            edit_tick = now_tick;
            continue;
        }

        if (now_tick - display_tick > pdMS_TO_TICKS(500)) {
            cur_display_num = (cur_display_num == ' ') ? num[pos] : ' ';
            hc595_display_num_pos(cur_display_num, pos, false);
            display_tick = now_tick;
        }

        if (now_tick - edit_tick > pdMS_TO_TICKS(ADJUST_WAIT_TIME_MS)) {
            /* more than ADJUST_WAIT_TIME_MS no operation, exit. */
            return ADJUST_RES_OPT_TIMEOUT;
        }
    }

    uint16_t new_value = 0;

    new_value = (num[0] - '0') + (num[1] - '0') * 10 + (num[2] - '0') * 100 + (num[3] - '0') * 1000;

    if (new_value > max_val) {
        new_value = max_val;
    } else if (new_value < min_val) {
        new_value = min_val;
    }

    now_tick = xTaskGetTickCount();
    display_tick = now_tick;
    edit_tick = now_tick;

    int display_cnt = 0;
    while (1) {
        now_tick = xTaskGetTickCount();
        if (now_tick - display_tick < pdMS_TO_TICKS(1000)) {
            /* display 1 second name first */
            hc595_display_str(name);
        } else if (display_cnt < 10) {
            /* toggle display new value */
            (display_cnt % 2 == 0) ? hc595_display_uint16(new_value) : hc595_display_off();
            display_cnt++;
        } else {
            display_tick = now_tick;
            display_cnt = 0;
        }

        key_event_bits = xEventGroupWaitBits(key_event, KEY_BUTTON | KEY_ENCODER, pdTRUE, pdFALSE, pdMS_TO_TICKS(500));

        if (key_event_bits & KEY_BUTTON) {
            return ADJUST_RES_CANCELED;
        }
        if (key_event_bits & KEY_ENCODER) {
            return new_value;
        }
        if (xTaskGetTickCount() - edit_tick > pdMS_TO_TICKS(ADJUST_WAIT_TIME_MS)) {
            /* more than ADJUST_WAIT_TIME_MS no operation, exit. */
            return ADJUST_RES_OPT_TIMEOUT;
        }
    }
}

/**
 * @brief GPIO external interrupt callback. Detects button and encoder key
 *        presses with debounce and signals the key event group.
 *
 * @param GPIO_Pin Pin that triggered the interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t button_down_time;
    static uint32_t enc_key_down_time;
    static beep_data_t beep = { .times = 1, .on_period = 100, .off_period = 0 };

    if (GPIO_Pin == BUTTON_Pin) {
        if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
            xQueueOverwriteFromISR(g_beep_queue, &beep, NULL);
            button_down_time = HAL_GetTick();
        } else if (HAL_GetTick() - button_down_time > PRESS_DURATION_MS) {
            xEventGroupSetBitsFromISR(key_event, KEY_BUTTON, NULL);
        }
    } else if (GPIO_Pin == ENC_KEY_Pin) {
        if (HAL_GPIO_ReadPin(ENC_KEY_GPIO_Port, ENC_KEY_Pin) == GPIO_PIN_RESET) {
            xQueueOverwriteFromISR(g_beep_queue, &beep, NULL);
            enc_key_down_time = HAL_GetTick();
        } else if (HAL_GetTick() - enc_key_down_time > PRESS_DURATION_MS) {
            xEventGroupSetBitsFromISR(key_event, KEY_ENCODER, NULL);
        }
    }
}