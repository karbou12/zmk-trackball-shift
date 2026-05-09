/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_trackball_direction_detection

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>

#include <zmk/behavior.h>

#include "../trackball_shift.h"

LOG_MODULE_DECLARE(trackball_shift, CONFIG_TRACKBALL_SHIFT_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_trackball_direction_detection_binding_pressed(struct zmk_behavior_binding *binding,
                                                            struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    tb_set_direction_detection_active(true);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_trackball_direction_detection_binding_released(struct zmk_behavior_binding *binding,
                                                             struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    tb_set_direction_detection_active(false);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api trackball_direction_detection_driver_api = {
    .binding_pressed = on_trackball_direction_detection_binding_pressed,
    .binding_released = on_trackball_direction_detection_binding_released,
};

#define TRACKBALL_DIRECTION_DETCTION_ROT_INST(n)                                \
    BEHAVIOR_DT_INST_DEFINE(n,                                                  \
                            NULL,                                               \
                            NULL,                                               \
                            NULL,                                               \
                            NULL,                                               \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,   \
                            &trackball_direction_detection_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_DIRECTION_DETCTION_ROT_INST)

#endif
