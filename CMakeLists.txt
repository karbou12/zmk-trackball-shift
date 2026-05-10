# Copyright (c) 2026 Tano Karbou (github: karbou12 / X: @karbou_12)
# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_TRACKBALL_SHIFT)
    zephyr_library()

    if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
        zephyr_include_directories(
            ${APPLICATION_SOURCE_DIR}/include
            include
        )
        zephyr_library_sources(
            src/trackball_shift.c
            src/behaviors/behavior_trackball_shift_rotation.c
            src/behaviors/behavior_trackball_direction_detection.c
            src/input_processors/input_processors_trackball_shift.c
            src/input_processors/input_processors_trackball_direction_detection.c
        )
    endif()
endif()
