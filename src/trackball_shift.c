/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include "trackball_shift.h"

LOG_MODULE_REGISTER(trackball_shift, CONFIG_TRACKBALL_SHIFT_LOG_LEVEL);

struct trackball_shift_data {
    // struct k_mutex lock;
    uint8_t direction_angle_degree;
    uint16_t device_angle_degree;
    int32_t  sin_value;
    int32_t  cos_value;
};

static struct trackball_shift_data tb_data = {
    .direction_angle_degree = 45,
    .device_angle_degree = 0,
    .sin_value = 0,
    .cos_value = 0,
};

static uint8_t clamp_step_angle_degree(const uint8_t step_angle_degree, const uint8_t min_angle, const uint8_t max_angle) {
    uint8_t clamped_angle = step_angle_degree;

    if (clamped_angle < min_angle) {
        clamped_angle = min_angle;
    } else if (max_angle < clamped_angle) {
        clamped_angle = max_angle;
    }

    if (clamped_angle != 0) {
        while (360 % clamped_angle != 0) {
            clamped_angle++;
        }
    }

    if (step_angle_degree != clamped_angle) {
        LOG_WRN("angle is changed from %u to %u [degrees].", step_angle_degree, clamped_angle);
    }

    return clamped_angle;
}

static uint16_t clamp_device_angle_degree(const uint16_t device_angle_degree, const uint8_t step_angle) {
    uint16_t clamped_angle = device_angle_degree;

    if (clamped_angle == 0) {
        return clamped_angle;
    } else if (clamped_angle < step_angle) {
        clamped_angle = step_angle;
    } else {
        const uint16_t mod = clamped_angle % step_angle;
        if (mod != 0) {
            clamped_angle = step_angle * mod;
        }
    }

    if (device_angle_degree != clamped_angle) {
        LOG_WRN("angle is changed from %u to %u [degrees].", device_angle_degree, clamped_angle);
    }

    return clamped_angle;
}

static void get_sin_cos_value(const uint16_t degree, int32_t* sin_value, int32_t* cos_value) {
    // normalize sin table with int16_t max
    static const int32_t sin_tbl[] = {
            0,   572,  1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,
         5690,  6252,  6813,  7371,  7927,  8481,  9032,  9580, 10126, 10668,
        11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886,
        16383, 16876, 17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621,
        21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964, 24351, 24730,
        25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087,
        28377, 28659, 28932, 29196, 29451, 29697, 29934, 30162, 30381, 30591,
        30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
        32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762,
        32767,
    };

    // when degree is 0
    // x' = x cos - y sin
    // y' = x sin + y cos

    if (degree < 90) {
        const uint8_t sin_index = degree;
        const uint8_t cos_index  = 90 - degree;
        *sin_value = sin_tbl[sin_index];
        *cos_value = sin_tbl[cos_index];
    } else if (degree < 180) {
        const uint8_t sin_index = 180 - degree;
        const uint8_t cos_index  = degree - 90;
        *sin_value = sin_tbl[sin_index];
        *cos_value = - sin_tbl[cos_index];
    } else if (degree < 270) {
        const uint8_t sin_index = degree - 180;
        const uint8_t cos_index  = 270 - degree;
        *sin_value = - sin_tbl[sin_index];
        *cos_value = - sin_tbl[cos_index];
    } else  {
        const uint8_t sin_index = 360 - degree;
        const uint8_t cos_index  = degree - 270;
        *sin_value = - sin_tbl[sin_index];
        *cos_value = sin_tbl[cos_index];
    }
}

void tb_set_direction_angle_degree(const uint8_t direction_angle_degree) {
    tb_data.direction_angle_degree = clamp_step_angle_degree(direction_angle_degree, 3, 45);
}

void tb_set_device_angle_degree(const uint16_t device_angle_degree) {
    tb_data.device_angle_degree = clamp_device_angle_degree(device_angle_degree, tb_data.direction_angle_degree);
    get_sin_cos_value(tb_data.device_angle_degree, &tb_data.sin_value, &tb_data.cos_value);

    LOG_INF("[device angle:%u][sin_val:%u][cos_val:%u]", tb_data.device_angle_degree, tb_data.sin_value, tb_data.cos_value);
}

void tb_rotate_point(const int16_t raw_x, const int16_t raw_y,
                     int16_t* x, int16_t* y) {
    const int32_t SCALER = 32767;

    // when degree is 0
    // x' = x cos - y sin
    // y' = x sin + y cos
    *x = (int16_t)((raw_x * tb_data.cos_value - raw_y * tb_data.sin_value) / SCALER);
    *y = (int16_t)((raw_x * tb_data.sin_value + raw_y * tb_data.cos_value) / SCALER);

    LOG_DBG("[device rotation angle:%u] [x:y] [%d:%d] -> [%d:%d]",
            tb_data.device_angle_degree, raw_x, raw_y, *x, *y);
}
