/*
 * Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_trackball_shift_rotation

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>

#include <zmk/behavior.h>

#include "../trackball_shift.h"
#include <dt-bindings/zmk/trackball_shift_rotation.h>

LOG_MODULE_DECLARE(trackball_shift, CONFIG_TRACKBALL_SHIFT_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_value_metadata no_arg_values[] = {
    {
        .display_name = "Clockwise",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = TB_ROT_CW,
    },
    {
        .display_name = "Counter-Clockwise",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = TB_ROT_CCW,
    },
};

static const struct behavior_parameter_metadata_set no_args_set = {
    .param1_values = no_arg_values,
    .param1_values_len = ARRAY_SIZE(no_arg_values),
};

static const struct behavior_parameter_metadata_set metadata_sets[] = {no_args_set};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(metadata_sets),
    .sets = metadata_sets,
};
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

struct behavior_trackball_shift_rotation_config {
    uint8_t step_angle_deg;
};

static int behavior_trackball_shift_rotation_init(const struct device *dev) {
    return 0;
};

static int on_trackball_shift_rotation_binding_pressed(struct zmk_behavior_binding *binding,
                                                       struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_trackball_shift_rotation_config *config = dev->config;

    switch (binding->param1) {
        case TB_ROT_CW:
            LOG_INF("ROT_CW");
            tb_rotate_device_with_step(config->step_angle_deg, true);
            return ZMK_BEHAVIOR_OPAQUE;

        case TB_ROT_CCW:
            LOG_INF("ROT_CCW");
            tb_rotate_device_with_step(config->step_angle_deg, false);
            return ZMK_BEHAVIOR_OPAQUE;

        default:
            LOG_ERR("Unknown TB_ROT command: %d", binding->param1);
            return -ENOTSUP;
    };
}

static int on_trackball_shift_rotation_binding_released(struct zmk_behavior_binding *binding,
                                                        struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api trackball_shift_rotation_driver_api = {
    .binding_pressed = on_trackball_shift_rotation_binding_pressed,
    .binding_released = on_trackball_shift_rotation_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

#define TRACKBALL_SHIFT_ROT_INST(n)                                             \
    static const struct behavior_trackball_shift_rotation_config                \
        behavior_trackball_shift_rotation_config_##n = {                        \
        .step_angle_deg = DT_INST_PROP_OR(n, step_angle_deg, 45),               \
    };                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n,                                                  \
                            &behavior_trackball_shift_rotation_init,            \
                            NULL,                                               \
                            NULL,                                               \
                            &behavior_trackball_shift_rotation_config_##n,      \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,   \
                            &trackball_shift_rotation_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_SHIFT_ROT_INST)

#endif
