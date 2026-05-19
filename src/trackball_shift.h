/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

extern void tb_init();
extern void tb_set_device_angle_deg(const uint16_t device_angle_deg);
extern void tb_set_direction_detection_active(const bool is_active);
extern bool tb_is_direction_detection_active();

extern int tb_rotate_point(const uint8_t type, const uint16_t code, int32_t* value);
extern void tb_rotate_device_with_step(const uint8_t step_angle_deg, const bool is_cw);
extern int tb_detect_direction(const uint8_t type, const uint16_t code, int32_t* value);
