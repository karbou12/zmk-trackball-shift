/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_trackball_angle_changed {
    uint16_t angle_deg;
};

ZMK_EVENT_DECLARE(zmk_trackball_angle_changed);
