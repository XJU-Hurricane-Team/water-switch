/**
 * Copyright (c) 2026, Deadline039
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HC595_H
#define HC595_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Write a single character to a specific digit position on the display.
 *
 * @param ch       Character to display ('0'-'9', 'A'-'F', 'a'-'f', 'r', 'R , 
 *                 'u', 'U', 'p', 'P, 'o', 'O', 'n', 'N' , '-', or ' ' for blank).
 * @param position  Digit position (0 = leftmost, 3 = rightmost). Values >= 4
 *                  are ignored.
 * @param show_dot  If true, the decimal-point segment of this digit is lit.
 */
void hc595_display_num_pos(char ch, uint8_t position, bool show_dot);

/**
 * @brief Turn off all four digits (all segments high / inactive).
 */
void hc595_display_off(void);

/**
 * @brief Display a 16-bit unsigned integer on the 4-digit display,
 *        right-aligned.
 *
 * @param number Value to display (0-9999). Values > 9999 wrap via modulo.
 */
void hc595_display_uint16(uint16_t number);

/**
 * @brief Write a right-aligned string to the display buffer.
 *        A trailing '.' after a character enables the decimal-point dot for
 *        that digit (e.g. "Err."). Excess characters are discarded; unused
 *        leading positions are padded with spaces.
 *
 * @param str Null-terminated string to display (max 4 visible characters).
 */
void hc595_display_str(const char *str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HC595_H */
