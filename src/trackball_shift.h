/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

extern void tb_set_device_angle_degree(const uint16_t device_angle_degree);
extern void tb_init_detection_data(const uint16_t distance_threshold, const uint16_t sample_time_ms);
extern void tb_set_direction_detection_active(const bool is_active);
extern bool tb_is_direction_detection_active();

extern void tb_rotate_point(const int16_t raw_x, const int16_t raw_y, int16_t* x, int16_t* y);
extern void tb_rotate_device_with_step(const uint8_t step_angle_degree, const bool is_cw);
extern void tb_detect_direction(const int16_t value, const bool is_y_value);
