/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_processor_trackball_direction_detection

#include <zephyr/device.h>
#include <drivers/input_processor.h>
#include <zephyr/logging/log.h>
#include "../trackball_shift.h"

LOG_MODULE_DECLARE(trackball_shift, CONFIG_TRACKBALL_SHIFT_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int trackball_direction_detection_init(const struct device *dev) {
    ARG_UNUSED(dev);
    tb_init();

    return 0;
}

static int trackball_direction_detection_handle_event(const struct device *dev, struct input_event *event,
                                                      uint32_t param1, uint32_t param2,
                                                      struct zmk_input_processor_state *state) {
    ARG_UNUSED(dev);
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (event->code == INPUT_REL_X) {
        tb_detect_direction(event->value, false);

    } else if (event->code == INPUT_REL_Y) {
        tb_detect_direction(event->value, true);

    } else {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    event->value = 0;

    return 0;
}

// API struct
static const struct zmk_input_processor_driver_api trackball_direction_detection_driver_api = {
    .handle_event = trackball_direction_detection_handle_event,
};

#define TRACKBALL_DIRECTION_DETECTION_INST(n)                                     \
    DEVICE_DT_INST_DEFINE(n,                                                      \
                          trackball_direction_detection_init,                     \
                          NULL,                                                   \
                          NULL,                                                   \
                          NULL,                                                   \
                          POST_KERNEL,                                            \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                    \
                          &trackball_direction_detection_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_DIRECTION_DETECTION_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
