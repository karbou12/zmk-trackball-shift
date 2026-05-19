/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_processor_trackball_shift

#include <zephyr/device.h>
#include <drivers/input_processor.h>
#include <zephyr/logging/log.h>
#include "../trackball_shift.h"

LOG_MODULE_DECLARE(trackball_shift, CONFIG_TRACKBALL_SHIFT_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct trackball_shift_config {
    uint16_t default_device_angle_deg;
};

static int trackball_shift_init(const struct device *dev) {
    tb_init();

    struct trackball_shift_config *config = (struct trackball_shift_config *)dev->config;

    tb_set_device_angle_deg(config->default_device_angle_deg);

    return 0;
}

static int trackball_shift_handle_event(const struct device *dev, struct input_event *event,
                                        uint32_t param1, uint32_t param2,
                                        struct zmk_input_processor_state *state) {
    ARG_UNUSED(dev);
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    if (tb_is_direction_detection_active()) {
        return tb_detect_direction(event->type, event->code, &event->value);
    } else {
        return tb_rotate_point(event->type, event->code, &event->value);
    }

    return 0;
}

// API struct
static const struct zmk_input_processor_driver_api trackball_shift_driver_api = {
    .handle_event = trackball_shift_handle_event,
};

#define TRACKBALL_SHIFT_INST(n)                                                             \
    static struct trackball_shift_config trackball_shift_config_##n = {                     \
        .default_device_angle_deg = DT_INST_PROP_OR(n, default_device_angle_deg, 0),  \
    };                                                                                      \
    DEVICE_DT_INST_DEFINE(n,                                                                \
                          trackball_shift_init,                                             \
                          NULL,                                                             \
                          NULL,                                                             \
                          &trackball_shift_config_##n,                                      \
                          POST_KERNEL,                                                      \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                              \
                          &trackball_shift_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_SHIFT_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
