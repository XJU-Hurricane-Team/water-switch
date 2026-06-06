/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "includes.h"

#define KEY_BUTTON        (1U << 0)
#define KEY_ENCODER       (1U << 1)

#define PRESS_DURATION_MS 20

static EventGroupHandle_t key_event;
TaskHandle_t key_task_handle;

static uint16_t adjust_limit(uint16_t current_num);

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
            PUMP_TOGGLE();
        }
        if (key_event_bits & KEY_ENCODER) {
            uint16_t new_limit = 0;
#if MODE_CONF == PUMPING_MODE
            uint16_t last_upper_limit = g_upper_limit;
#endif /* MODE_CONF == PUMPING_MODE */

            encoder_start();
            g_threshold_adj = true;

            new_limit = adjust_limit(g_upper_limit);

            /* update when vaild */
            g_upper_limit = new_limit != 0 ? new_limit : g_upper_limit;

#if MODE_CONF == PUMPING_MODE
            new_limit = adjust_limit(g_lower_limit);
            if (new_limit >= g_upper_limit) {
                /* invalid, restore */
                g_upper_limit = last_upper_limit;
            } else if (new_limit != 0) {
                /* update when vaild */
                g_lower_limit = new_limit;
            }
#endif /* MODE_CONF == PUMPING_MODE */

            encoder_stop();
            g_threshold_adj = false;
            adc_save_limit();
        }
    }
}

/**
 * @brief Digit-by-digit limit adjustment using the rotary encoder.
 *        The user cycles through digit positions, adjusts each digit, and
 *        confirms or cancels the new value.
 *
 * @param current_num Current limit value to start adjusting from.
 * @return The new limit value (clamped to MIN/MAX water level), or 0 if
 *         cancelled.
 */
static uint16_t adjust_limit(uint16_t current_num)
{
    EventBits_t key_event_bits;

    int pos = 3;
    char num[4];

    hc595_display_uint16(current_num);
    for (int i = 0; i < 4; i++) {
        num[i] = (current_num % 10) + '0';
        current_num /= 10;
    }

    char cur_display_num = ' ';
    char last_display_num;

    TickType_t last_display_tick = xTaskGetTickCount();
    uint16_t enc_cnt = encoder_get_count();
    uint16_t last_enc_cnt = enc_cnt;

    do {
        key_event_bits = xEventGroupWaitBits(key_event, KEY_BUTTON | KEY_ENCODER, pdTRUE, pdFALSE, pdMS_TO_TICKS(150));
        if (key_event_bits & KEY_BUTTON) {
            return 0;
        }
        if (key_event_bits & KEY_ENCODER) {
            hc595_display_num_pos(num[pos], pos, false);
            pos--;
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
            continue;
        }

        if (xTaskGetTickCount() - last_display_tick > pdMS_TO_TICKS(500)) {
            cur_display_num = (cur_display_num == ' ') ? num[pos] : ' ';
            hc595_display_num_pos(cur_display_num, pos, false);
            last_display_tick = xTaskGetTickCount();
        }
    } while (pos >= 0);

    uint16_t new_value = 0;

    new_value = (num[0] - '0') + (num[1] - '0') * 10 + (num[2] - '0') * 100 + (num[3] - '0') * 1000;

    if (new_value > WATER_MAX_LEVEL) {
        new_value = WATER_MAX_LEVEL;
    } else if (new_value < WATER_MIN_LEVEL) {
        new_value = WATER_MIN_LEVEL;
    }

    /* false=off, true=display number */
    bool num_toggle = false;
    while (1) {
        if (num_toggle) {
            hc595_display_uint16(new_value);
        } else {
            hc595_display_off();
        }

        key_event_bits = xEventGroupWaitBits(key_event, KEY_BUTTON | KEY_ENCODER, pdTRUE, pdFALSE, pdMS_TO_TICKS(500));
        if (key_event_bits & KEY_BUTTON) {
            return 0;
        }
        if (key_event_bits & KEY_ENCODER) {
            return new_value;
        }
        num_toggle = !num_toggle;
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

    if (GPIO_Pin == BUTTON_Pin) {
        if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
            xSemaphoreGiveFromISR(beep_sem, NULL);
            button_down_time = HAL_GetTick();
        } else if (HAL_GetTick() - button_down_time > PRESS_DURATION_MS) {
            xEventGroupSetBitsFromISR(key_event, KEY_BUTTON, NULL);
        }
    } else if (GPIO_Pin == ENC_KEY_Pin) {
        if (HAL_GPIO_ReadPin(ENC_KEY_GPIO_Port, ENC_KEY_Pin) == GPIO_PIN_RESET) {
            xSemaphoreGiveFromISR(beep_sem, NULL);
            enc_key_down_time = HAL_GetTick();
        } else if (HAL_GetTick() - enc_key_down_time > PRESS_DURATION_MS) {
            xEventGroupSetBitsFromISR(key_event, KEY_ENCODER, NULL);
        }
    }
}