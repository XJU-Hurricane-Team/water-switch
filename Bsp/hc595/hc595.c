/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#include "hc595.h"

#include <spi.h>

/**
 * Seven-segment display lookup table for hexadecimal digits (0-F), minus sign, and space.
 * Each byte represents the segment pattern where each bit controls one LED segment.
 */
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

/**
 * Position/digit selection table for the 4-digit display.
 * Each byte represents which digit position is active (low bit = active).
 */
static const uint8_t position_table[4] = { 0xF7, 0xFB, 0xFD, 0xFE };

/**
 * Buffer storing the display data for each of the 4 digit positions.
 * Format: [position_bits << 8 | segment_bits]
 */
static uint16_t digital_data[4];

/**
 * Macros to control the Register Clock (RCLK) pin for HC595 latch.
 * RCLK triggers the latch to transfer shift register data to output registers.
 */
#define HC595_RCLK_HIGH() (HC595_RCLK_GPIO_Port->BSRR = HC595_RCLK_Pin)
#define HC595_RCLK_LOW()  (HC595_RCLK_GPIO_Port->BRR = HC595_RCLK_Pin)

/**
 * Display a single digit at the specified position on the 7-segment display.
 *
 * @param num       Character representing the digit ('0'-'9', 'a'-'f', '-', or ' ')
 * @param position  Display position (0-3, left to right)
 * @param show_dot  If true, enables the decimal point dot for this digit
 */
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

/**
 * Display a 16-bit unsigned integer on the 4-digit display.
 * The number is displayed right-aligned (units place on the right).
 *
 * @param number    16-bit unsigned integer to display (0-9999)
 */
void hc595_display_uint16(uint16_t number)
{
    uint16_t num = number;
    for (int i = 0; i < 4; i++) {
        hc595_display_num_pos((num % 10) + '0', i, false);
        num /= 10;
    }
}

/**
 * Turn off all digits on the display by setting all segments to inactive (high).
 *
 */
void hc595_display_off(void)
{
    for (int i = 0; i < 4; i++) {
        digital_data[i] = (position_table[i] << 8) | 0xFF;
    }
}

/**
 * Internal timer tick function for display scanning.
 * This function is called periodically by the timer interrupt to cyclically
 * update each of the 4 digit positions, creating a multiplexed display effect.
 * Transmits the current digit data via SPI to the HC595 shift register and
 * latches it using the RCLK pin.
 */
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

/**
 * HAL timer period elapsed callback handler.
 * Called by the HAL driver when TIM4 period completes.
 * Triggers the display scanning routine to update the current digit.
 *
 * @param htim      Pointer to the timer handle that triggered this callback
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        hc595_display_tick();
    }
}