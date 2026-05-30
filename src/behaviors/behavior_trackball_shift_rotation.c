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
static const struct behavior_parameter_value_metadata rotate_param1_values[] = {
    {
        .display_name = "Rotate Clockwise",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = TB_CW_CMD,
    },
    {
        .display_name = "Rotate Counter-Clockwise",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = TB_CCW_CMD,
    },
};

static const struct behavior_parameter_value_metadata rotate_param2_values[] = {
    {
        .display_name = "Rotation Angle",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE,
        .range = {.min = 3, .max = 45},
    },
};

static const struct behavior_parameter_metadata_set rotate_metadata_set = {
    .param1_values = rotate_param1_values,
    .param1_values_len = ARRAY_SIZE(rotate_param1_values),
    .param2_values = rotate_param2_values,
    .param2_values_len = ARRAY_SIZE(rotate_param2_values),
};

static const struct behavior_parameter_value_metadata set_angle_param1_values[] = {
    {
        .display_name = "Set Device Angle",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = TB_SET_CMD,
    },
};

static const struct behavior_parameter_value_metadata set_angle_param2_values[] = {
    {
        .display_name = "Angle",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE,
        .range = {.min = 0, .max = 360},
    },
};

static const struct behavior_parameter_metadata_set set_angle_metadata_set = {
    .param1_values = set_angle_param1_values,
    .param1_values_len = ARRAY_SIZE(set_angle_param1_values),
    .param2_values = set_angle_param2_values,
    .param2_values_len = ARRAY_SIZE(set_angle_param2_values),
};

static const struct behavior_parameter_metadata_set metadata_sets[] = {rotate_metadata_set,
                                                                       set_angle_metadata_set};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(metadata_sets),
    .sets = metadata_sets,
};
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static int on_trackball_shift_rotation_binding_pressed(struct zmk_behavior_binding *binding,
                                                       struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    switch (binding->param1) {
        case TB_CW_CMD:
            LOG_INF("Rotate Clockwise");
            tb_rotate_device_with_step(binding->param2, true);
            return ZMK_BEHAVIOR_OPAQUE;

        case TB_CCW_CMD:
            LOG_INF("Rotate Counter Clockwise");
            tb_rotate_device_with_step(binding->param2, false);
            return ZMK_BEHAVIOR_OPAQUE;

        case TB_SET_CMD:
            LOG_INF("Set Device Angle");
            tb_set_device_angle_deg(binding->param2);
            return ZMK_BEHAVIOR_OPAQUE;

        default:
            LOG_ERR("Unknown trackball shift command: %d", binding->param1);
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
    BEHAVIOR_DT_INST_DEFINE(n,                                                  \
                            NULL,                                               \
                            NULL,                                               \
                            NULL,                                               \
                            NULL,                                               \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,   \
                            &trackball_shift_rotation_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_SHIFT_ROT_INST)

#endif
