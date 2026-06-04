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

void hc595_display_num_pos(char num, uint8_t position, bool show_dot);
void hc595_display_off(void);
void hc595_display_uint16(uint16_t number);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HC595_H */
