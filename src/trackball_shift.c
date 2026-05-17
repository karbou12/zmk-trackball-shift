/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.1415926536
#endif
#include "trackball_shift.h"
#include <dt-bindings/zmk/trackball_shift_rotation.h>

LOG_MODULE_REGISTER(trackball_shift, CONFIG_TRACKBALL_SHIFT_LOG_LEVEL);

struct trackball_shift_data {
    struct k_mutex lock;

    uint8_t  direction_angle_deg;
    const uint16_t rotation_sample_time_ms;
    const uint32_t direction_squared_distance_threshold;
    const uint16_t direction_sample_time_ms;

    uint16_t device_angle_deg;
    int32_t  sin_value;
    int32_t  cos_value;

    bool     is_detected;
    bool     is_detection_active;
};

static struct trackball_shift_data tb_data = {
    .direction_angle_deg = CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG,
    .rotation_sample_time_ms = CONFIG_ZMK_TRACKBALL_SHIFT_ROTATION_SAMPLE_TIME_MS,
    .direction_squared_distance_threshold = CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_DISTANCE_THRESHOLD * CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_DISTANCE_THRESHOLD,
    .direction_sample_time_ms = CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_SAMPLE_TIME_MS,
    .device_angle_deg = 0,
    .sin_value = 0,
    .cos_value = 0,
    .is_detected = false,
    .is_detection_active = false,
};

static uint16_t clamp_angle_deg(const int16_t angle_deg) {
    const int16_t mod = angle_deg % 360;
    return (mod >= 0) ? mod : mod + 360;
}

static uint8_t clamp_step_angle_deg(const uint8_t step_angle_deg, const uint8_t min_angle, const uint8_t max_angle) {
    uint8_t clamped_angle = step_angle_deg;

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

    if (step_angle_deg != clamped_angle) {
        LOG_WRN("angle is changed from %u to %u [degs].", step_angle_deg, clamped_angle);
    }

    return clamped_angle;
}

static uint16_t clamp_device_angle_deg(const uint16_t device_angle_deg, const uint8_t step_angle) {
    uint16_t clamped_angle = device_angle_deg;

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

    if (device_angle_deg != clamped_angle) {
        LOG_WRN("angle is changed from %u to %u [degs].", device_angle_deg, clamped_angle);
    }

    return clamped_angle;
}

static void get_sin_cos_value(const uint16_t angle_deg, int32_t* sin_value, int32_t* cos_value) {
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

    // when angle_deg is 0
    // x' = x cos - y sin
    // y' = x sin + y cos

    if (angle_deg < 90) {
        const uint8_t sin_index = angle_deg;
        const uint8_t cos_index  = 90 - angle_deg;
        *sin_value = sin_tbl[sin_index];
        *cos_value = sin_tbl[cos_index];
    } else if (angle_deg < 180) {
        const uint8_t sin_index = 180 - angle_deg;
        const uint8_t cos_index  = angle_deg - 90;
        *sin_value = sin_tbl[sin_index];
        *cos_value = - sin_tbl[cos_index];
    } else if (angle_deg < 270) {
        const uint8_t sin_index = angle_deg - 180;
        const uint8_t cos_index  = 270 - angle_deg;
        *sin_value = - sin_tbl[sin_index];
        *cos_value = - sin_tbl[cos_index];
    } else  {
        const uint8_t sin_index = 360 - angle_deg;
        const uint8_t cos_index  = angle_deg - 270;
        *sin_value = - sin_tbl[sin_index];
        *cos_value = sin_tbl[cos_index];
    }
}

static void rotate_device(const bool is_cw) {
    const uint16_t next_device_angle = is_cw ? tb_data.device_angle_deg + tb_data.direction_angle_deg
                                             : tb_data.device_angle_deg - tb_data.direction_angle_deg;

    tb_set_device_angle_deg(clamp_angle_deg(next_device_angle));
}

void tb_init() {
    static bool is_init = false;
    if (is_init) {
        return;
    }
    is_init = true;

    k_mutex_init(&tb_data.lock);
    tb_data.direction_angle_deg = clamp_step_angle_deg(tb_data.direction_angle_deg, 3, 45);
}

void tb_set_device_angle_deg(const uint16_t device_angle_deg) {
    tb_data.device_angle_deg = clamp_device_angle_deg(device_angle_deg, tb_data.direction_angle_deg);
    get_sin_cos_value(tb_data.device_angle_deg, &tb_data.sin_value, &tb_data.cos_value);

    LOG_INF("[device angle:%u][sin_val:%u][cos_val:%u]", tb_data.device_angle_deg, tb_data.sin_value, tb_data.cos_value);
}

void tb_set_direction_detection_active(const bool is_active) {
    if (k_mutex_lock(&tb_data.lock, K_FOREVER) == 0) {
        tb_data.is_detection_active = is_active;
        k_mutex_unlock(&tb_data.lock);
    }
}

bool tb_is_direction_detection_active() {
    bool is_active = false;
    if (k_mutex_lock(&tb_data.lock, K_FOREVER) == 0) {
        is_active = tb_data.is_detection_active;
        k_mutex_unlock(&tb_data.lock);
    }
    return is_active;
}


void tb_rotate_point(const int16_t raw_x, const int16_t raw_y,
                     int16_t* x, int16_t* y) {
    tb_data.is_detected = false;

    const int32_t SCALER = 32767;

    // when angle is 0
    // x' = x cos - y sin
    // y' = x sin + y cos
    *x = (int16_t)((raw_x * tb_data.cos_value - raw_y * tb_data.sin_value) / SCALER);
    *y = (int16_t)((raw_x * tb_data.sin_value + raw_y * tb_data.cos_value) / SCALER);

    LOG_DBG("[device rotation angle:%u] [x:y] [%d:%d] -> [%d:%d]",
            tb_data.device_angle_deg, raw_x, raw_y, *x, *y);
}

void tb_rotate_device_with_step(const uint8_t step_angle_deg, const bool is_cw) {
    static int8_t acc_angle_deg = 0;
    static int8_t total = 0;

    static int64_t prev_time = 0;
    int64_t curr_time = k_uptime_get();
    const int64_t diff_time = curr_time - prev_time;
    if ((prev_time == 0) || (diff_time > tb_data.rotation_sample_time_ms)) {
        prev_time = curr_time;
        acc_angle_deg = 0;
        total = 0;
    }

    const uint8_t clamped_step_angle = clamp_step_angle_deg(step_angle_deg, 3, 45);
    acc_angle_deg += is_cw ? clamped_step_angle : - clamped_step_angle;
    LOG_DBG("%s %d -> %d\n", __FUNCTION__, step_angle_deg, acc_angle_deg);

    bool is_rotate = false;
    uint8_t count = 0;
    int8_t tmp_angle_deg = acc_angle_deg;

    if (acc_angle_deg > 0) {
        while (tmp_angle_deg >= tb_data.direction_angle_deg) {
            rotate_device(true);
            is_rotate = true;
            count++;
            total++;
            tmp_angle_deg -= tb_data.direction_angle_deg;
            if (tmp_angle_deg < 0) {
                tmp_angle_deg = 0;
            }
        }
    } else if (acc_angle_deg < 0) {
        while (tmp_angle_deg <= -tb_data.direction_angle_deg) {
            rotate_device(false);
            is_rotate = true;
            count++;
            total++;
            tmp_angle_deg += tb_data.direction_angle_deg;
            if (tmp_angle_deg > 0) {
                tmp_angle_deg = 0;
            }
        }
    }

    if (is_rotate) {
        LOG_DBG("count:%d, total:%d, remain angle:%d\n", count, total, tmp_angle_deg);
        acc_angle_deg = tmp_angle_deg;
        return;
    }
}

void tb_detect_direction(const int16_t value, const bool is_y_value) {
    if (tb_data.is_detected) {
        return;
    }

    static int16_t acc_x = 0;
    static int16_t acc_y = 0;

    static int64_t prev_time = 0;
    const int64_t curr_time = k_uptime_get();
    const int64_t diff_time = curr_time - prev_time;

    if ((prev_time == 0) || (diff_time > tb_data.direction_sample_time_ms * 2)) {
        LOG_DBG("detection begin at %lld", curr_time);
        prev_time = curr_time;

        if (is_y_value) {
            acc_y = value;
        } else {
            acc_x = value;
        }

        return;
    }

    if (is_y_value) {
        acc_y += value;
    } else {
        acc_x += value;
        return;
    }

    const uint32_t distance = acc_x * acc_x + acc_y * acc_y;

    if (diff_time < tb_data.direction_sample_time_ms) {
        LOG_DBG("under detection [dst:%d %d -> %u/%u] [time:%lld - %lld = %lld/%d]",
                acc_x, acc_y, distance, tb_data.direction_squared_distance_threshold,
                curr_time, prev_time, diff_time, tb_data.direction_sample_time_ms);

        if (distance < tb_data.direction_squared_distance_threshold) {
            return;
        }
    }

    const double radian = atan2(acc_y, acc_x);
    int16_t roll_forward_angle = (int16_t)(radian * 180 / M_PI);

    LOG_INF("timeout detection [dst:%d %d (deg:%d) -> %u/%u] [time:%lld - %lld = %lld/%d]",
            acc_x, acc_y, roll_forward_angle, distance, tb_data.direction_squared_distance_threshold,
            curr_time, prev_time, diff_time, tb_data.direction_sample_time_ms);

    prev_time = 0;
    acc_x = 0;
    acc_y = 0;

    if (distance < tb_data.direction_squared_distance_threshold) {
        return;
    }

    const int16_t roll_forward_angle_base = 270;
    const int16_t device_x_axis_angle = roll_forward_angle_base - roll_forward_angle;

    // e.g., there is 8 directions if direction angle is 45.
    // if detected angle is 0, it's direction index is 0, and the angle range of 0th direction is from -22.5 to 22.5.
    // to calculate direction of the detected angle easily, shift the range from 0 to 45 by adding 45/2: direction_angle_deg / 2.
    const int16_t shifted_angle_in_direction = device_x_axis_angle + (int16_t)(tb_data.direction_angle_deg / 2);
    const uint16_t clamped_angle = clamp_angle_deg(shifted_angle_in_direction);
    const uint8_t device_direction_index = (uint8_t)(clamped_angle / tb_data.direction_angle_deg);

    tb_set_device_angle_deg(device_direction_index * tb_data.direction_angle_deg);
    tb_data.is_detected = true;

    return;
}
