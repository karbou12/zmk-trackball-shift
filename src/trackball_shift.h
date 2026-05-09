/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

extern void tb_set_direction_angle_degree(const uint8_t direction_angle_degree);
extern void tb_set_device_angle_degree(const uint16_t device_angle_degree);

extern void tb_rotate_point(const int16_t raw_x, const int16_t raw_y, int16_t* x, int16_t* y);
