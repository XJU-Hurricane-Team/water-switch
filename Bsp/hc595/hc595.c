/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "hc595.h"

#include <spi.h>

static const uint8_t digital_table[] = {
    0xC0, /* 0 */
    0xF9, /* 1 */
    0xA4, /* 2 */
    0xB0, /* 3 */
    0x99, /* 4 */
    0x92, /* 5 */
    0x82, /* 6 */
    0xF8, /* 7 */
    0x80, /* 8 */
    0x90, /* 9 */
    0x88, /* A */
    0x83, /* B */
    0xC6, /* C */
    0xA1, /* D */
    0x86, /* E */
    0x8E, /* F */
    0xbf, /* - */
    0xFF, /*   */
};

static const uint8_t position_table[4] = { 0xF7, 0xFB, 0xFD, 0xFE };

static uint16_t digital_data[4];

#define HC595_RCLK_HIGH() (HC595_RCLK_GPIO_Port->BSRR = HC595_RCLK_Pin)
#define HC595_RCLK_LOW()  (HC595_RCLK_GPIO_Port->BRR = HC595_RCLK_Pin)

void hc595_display_num_pos(char num, uint8_t position, bool show_dot)
{
    if (position >= 4) {
        return;
    }

    uint8_t digital_num;

    switch (num) {
        case '0' ... '9':
            digital_num = digital_table[num - '0'];
            break;

        case 'a' ... 'f':
            digital_num = digital_table[num - 'a' + 10];
            break;

        case '-':
            digital_num = digital_table[16];
            break;

        case ' ':
            digital_num = digital_table[17];
            break;

        default:
            return;
    }
    if (show_dot == true) {
        digital_num &= ~(1 << 7);
    }
    digital_data[position] = (position_table[position] << 8) | digital_num;
}

void hc595_display_uint16(uint16_t number)
{
    uint16_t num = number;
    for (int i = 0; i < 4; i++) {
        hc595_display_num_pos((num % 10) + '0', i, false);
        num /= 10;
    }
}

void hc595_display_off(void)
{
    for (int i = 0; i < 4; i++) {
        digital_data[i] = (position_table[i] << 8) | 0xFF;
    }
}

static void hc595_display_tick(void)
{
    static uint32_t scan_idx;

    LL_SPI_TransmitData16(SPI1, digital_data[scan_idx]);
    while (LL_SPI_IsActiveFlag_BSY(SPI1))
        ;
    HC595_RCLK_LOW();
    HC595_RCLK_HIGH();
    HC595_RCLK_LOW();

    scan_idx++;
    scan_idx %= 4;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        hc595_display_tick();
    }
}