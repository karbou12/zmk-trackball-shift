# ZMK Module: zmk-feature-trackball_shift

A ZMK input processor module that detects the direction of a trackball device and rotates the XY input coordinates.
The module name is inspired by the "OctaShift" feature of the Nape Pro.

Note: I previously created a similar feature for the Nape Origin by modifying the PMW3610 driver. 
I have now extracted that feature into this standalone module and improved upon it.

## Features

- **Default Rotation:** Rotates XY input coordinates by a default angle defined in the configuration file.
- **Dynamic CW/CCW Rotation:** Supports clockwise (CW) and counter-clockwise (CCW) coordinate rotation via key presses or rotary encoders. The rotation angle is fully configurable.
- **Fixed Angle Selection:** Supports setting the device angle directly via behavior key presses. The angle can be configured using a behavior parameter.
- **Direction Detection:** Detects the physical orientation of the trackball device by rolling the ball from bottom to top while on a specific detection layer or holding a behavior key. The orientation resets when the device disconnects.

## Installation

Add this module to your ZMK firmware project by updating your `config/west.yml` file.

### 1. Update `west.yml`

Add this repository to the `remotes` and `projects` sections:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    # Add this remote
    - name: karbou12
      url-base: https://github.com/karbou12
  projects:
    - name: zmk
      remote: zmkfirmware
      import: app/west.yml
    # Add this project module
    - name: zmk-trackball-shift
      remote: karbou12
      revision: main
```

### 2. Basic Configuration

To enable the basic trackball shift functionality, include the header and define the input processor in your `.dtsi`, `.overlay`, or `.keymap` file:

```c
#include <input_processors/trackball_shift.dtsi>
#include <input_processors/trackball_direction_detection.dtsi>
#include <behaviors/trackball_shift_rotation.dtsi>
#include <behaviors/trackball_direction_detection.dtsi>

/ {
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        status = "okay";
        device = <&trackball>;

        // the trackball shift input processor
        input-processors = <&zip_trackball_shift>;

        // direction detection input processor
        trackball_shift_detection {
            layers = <1>;
            input-processors = <&zip_trackball_direction_detection>;
        };
    };
};

// (Optional) Set your initial physical device angle here
&zip_trackball_shift {
    default-device-angle-deg = <90>;
};

/ {
    keymap {
        compatible = "zmk,keymap";
        trackball_shift_layer {
            bindings = <
                // Dynamic CW/CCW Rotation Behavior
                &tbr TB_CCW_45     &tbr TB_CW_45

                // Fixed Angle Selection Behavior
                &tbr TB_0_DEG      &tbr TB_90_DEG

                // Direction Detection Behavior
                &tb_dd
            >;
        };
    };
};
```

## Trackball Shift Input Processor

### Overview

The trackball shift input processor is used to rotate XY input coordinates. The rotation angle can be determined by:
- A default angle specified in the devicetree.
- Dynamic CW/CCW rotation via key presses or rotary encoders.
- Direct configuration via behavior key presses.
- An angle detected via direction detection feature.

### Usage

When used, the trackball shift input processor takes no parameters. It must be used on all active layers except for the trackball direction detection layer.

#### Input Processor Ordering Rules:

- **General Recommendation:** It is recommended not to use xy transform input processors alongside this module; instead, configure the `default-device-angle-deg` property of `&zip_trackball_shift`.
- **Using XY Transform Processors:** If you choose to use xy transform input processors, `&zip_trackball_shift` **must be placed after** them in the sequence.
- **Layer-Specific XY Transformers:** If you add additional xy transform input processors on non-default layers, the initial base sequence (the default layer's xy transformers followed by `&zip_trackball_shift`) **must follow the exact same order** as the default layer. Any layer-specific xy transform processors must then be appended afterward.

```c
&zip_trackball_shift
```

#### Recommended Example

```c
#include <input_processors/trackball_shift.dtsi>
#include <input_processors/trackball_direction_detection.dtsi>

/ {
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        status = "okay";
        device = <&trackball>;

        // Not use xy transformer input processors
        input-processors = <
            &zip_trackball_shift
            &zip_mouse_gesture
        >;

        // Used in any layers except the detection layer
        // scroll transform can be used
        scroller {
            layers = <1>;
            input-processors = <
                &zip_trackball_shift
                &zip_xy_to_scroll_mapper &zip_scroll_scaler 1 10
                &zip_scroll_transform INPUT_TRANSFORM_X_INVERT
                &zip_inertia
            >;
        };

        // NOT used in the trackball direction detection layer
        direction_detection {
            layers = <2>;
            input-processors = <&zip_trackball_direction_detection>;
        };
    };
};

// To overwrite the default angle of the pre-defined instance `zip_trackball_shift`:
&zip_trackball_shift {
    default-device-angle-deg = <270>;
};
```

#### Example with Transform Input Processors

```c
#include <input_processors/trackball_shift.dtsi>
#include <input_processors/trackball_direction_detection.dtsi>

/ {
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        status = "okay";
        device = <&trackball>;

        // Must be placed AFTER base transform/scaler input processors
        input-processors = <
            &zip_xy_transform INPUT_TRANSFORM_XY_SWAP
            &zip_xy_transform INPUT_TRANSFORM_Y_INVERT
            &zip_trackball_shift
        >;

        // Used in any layers except the detection layer
        // Must maintain the exact same base sequence, then append additional processors
        scroller {
            layers = <1>;
            input-processors = <
                &zip_xy_transform INPUT_TRANSFORM_XY_SWAP
                &zip_xy_transform INPUT_TRANSFORM_Y_INVERT
                &zip_trackball_shift
                &zip_xy_transform INPUT_TRANSFORM_X_INVERT
                &zip_xy_to_scroll_mapper &zip_scroll_scaler 1 10
            >;
        };

        // NOT used in the trackball direction detection layer
        direction_detection {
            layers = <2>;
            input-processors = <&zip_trackball_direction_detection>;
        };
    };
};

// default-device-angle-deg will not be set because device angle is set by xy transform input processors.
```

### Pre-Defined Instances

One pre-defined instance of the trackball shift input processor is available:

| Reference | Description |
|---|---|
| `&zip_trackball_shift` | Rotates XY input coordinates. |

### User-Defined Instances

Users can define new instances of the trackball shift input processor if they want to target different codes. For example, if a device has two trackballs with different default physical angles, user-defined instances can be utilized.

#### Example

```c
/ {
    input_processors {
        zip_trackball_shift_peri: zip_trackball_shift_peri {
            compatible = "zmk,input-processor-trackball-shift";
            #input-processor-cells = <0>;
            default-device-angle-deg = <180>;
        };
    };
};
```

#### Compatible

The trackball shift input processor uses a `compatible` property `"zmk,input-processor-trackball-shift"`.

#### Standard Properties

- `#input-processor-cells` - required to be a constant value of `<0>`.

#### User Properties

- `default-device-angle-deg` - default device angle [deg] for trackball shift. If non-zero, this value must be a multiple of `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG` and a divisor of 360. The default value for the pre-defined instance is `0`.

### Configuration

#### Kconfig

Definition file: Kconfig

| Config | Type | Description | Default |
|---|---|---|---|
| `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG` | int | Angle [deg] of each direction for trackball shift. It must be between 3 and 45, and must be a divisor of 360. | 45 |

#### Devicetree

Applies to: `"zmk,input-processor-trackball-shift"`

Definition file: [dts/bindings/input_processors/zmk,input-processor-trackball-shift.yaml](https://github.com/karbou12/zmk-trackball-shift/blob/main/dts/bindings/input_processors/zmk,input-processor-trackball-shift.yaml)


| Property | Type | Description |
|---|---|---|
| `default-device-angle-deg` | int | Default device angle [deg] for trackball shift. If non-zero, this value must be a multiple of `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG` and a divisor of 360. |


## Dynamic CW/CCW Rotation

### Summary

The Dynamic CW/CCW Rotation behavior rotates XY input coordinates clockwise or counter-clockwise via key presses or rotary encoders.

### Trackball Shift Rotation Command Defines

The Trackball Shift Rotation command defines are provided through the [`dt-bindings/zmk/trackball_shift_rotation.h`](https://github.com/karbou12/zmk-trackball-shift/blob/main/include/dt-bindings/zmk/trackball_shift_rotation.h) header, which is added at the top of your keymap file:

```c
#include <dt-bindings/zmk/trackball_shift_rotation.h>
```

This will allow you to reference the actions defined in the header, such as `TB_CW_CMD`.

Here is a table describing the command for each define:


| Define | Action |
|---|---|
| `TB_CW_CMD` | Rotates XY input coordinates clockwise by the angle specified in parameter #2. |
| `TB_CCW_CMD` | Rotates XY input coordinates counter-clockwise by the angle specified in parameter #2. |
| `TB_CW_5` | Rotates XY input coordinates 5 [deg] clockwise. |
| `TB_CCW_5` | Rotates XY input coordinates 5 [deg] counter-clockwise. |
| `TB_CW_15` | Rotates XY input coordinates 15 [deg] clockwise. |
| `TB_CCW_15` | Rotates XY input coordinates 15 [deg] counter-clockwise. |
| `TB_CW_45` | Rotates XY input coordinates 45 [deg] clockwise. |
| `TB_CCW_45` | Rotates XY input coordinates 45 [deg] counter-clockwise. |

### Key Press

#### Behavior Bindings

- Reference: `&tbr`
- Parameter #1: The trackball shift rotation command define, e.g., `TB_CW_CMD`.
- Parameter #2: Step angle [deg] of the rotation.
    - Only applies to `TB_CW_CMD` and `TB_CCW_CMD`.
    - It is recommended to use the same value as `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG`.
    - If it is different from `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG`, the same threshold accumulation rules and algorithm as the encoder will be applied.

#### Examples

```c
#include <behaviors/trackball_shift_rotation.dtsi>

/ {
    keymap {
        compatible = "zmk,keymap";
        trackball_shift_layer {
            bindings = <
                &tbr TB_CCW_45     &tbr TB_CW_45
                &tbr TB_CCW_CMD 90 &tbr TB_CW_CMD 90
            >;
        };
    };
};
```

### Encoders

This behavior handles multiple ticks from a rotary encoder by accumulating the encoder's step angle within a specific sample time. 

If the step angle (parameter #2) is smaller than the threshold angle (`CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG`), the angles from each step are accumulated. Once the accumulated total exceeds the threshold angle within the window defined by `CONFIG_ZMK_TRACKBALL_SHIFT_ROTATION_SAMPLE_TIME_MS`, the system triggers a coordinate rotation equal to the threshold angle. If the threshold is not exceeded within this sample time, the accumulated value resets to zero.

- **Threshold:** `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG`
- **Sample Time:** `CONFIG_ZMK_TRACKBALL_SHIFT_ROTATION_SAMPLE_TIME_MS`
- **Step Angle:** Parameter #2 of the behavior

#### Pre-Defined Behavior Bindings: 45-degree rotation per step

- Reference: `&enc_tbr_45`
- Parameters: None.

#### Examples

```c
#include <behaviors/trackball_shift_rotation.dtsi>

/ {
    keymap {
        compatible = "zmk,keymap";
        trackball_shift_layer {
            sensor-bindings = <&enc_tbr_45>;
        };
    };
};
```

#### User-Defined Behavior Bindings

Users can define new behaviors for encoders if they want to customize the step angle or target different configurations.

For example, if a rotary encoder has 24 steps per full 360 [deg] rotation, each step equals a 15 [deg] angle. 
- If `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG` is set to `15`, the XY input coordinates will rotate 15 [deg] per 1 encoder step (15 [deg] physical rotation).
- If `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG` is set to `45`, the XY input coordinates will rotate 45 [deg] after 3 encoder steps (45 [deg] physical rotation).

##### Example

```c
#include <dt-bindings/zmk/behaviors.h>
#include <dt-bindings/zmk/trackball_shift_rotation.h>

/ {
    behaviors {
        enc_tbr_15: enc_tb15 {
            compatible = "zmk,behavior-sensor-rotate";
            #sensor-binding-cells = <0>;
            bindings = <&tbr TB_CCW_CMD 15>, <&tbr TB_CW_CMD 15>;
        };
    };

    keymap {
        compatible = "zmk,keymap";
        trackball_shift_layer {
            sensor-bindings = <&enc_tbr_15>;
        };
    };
};
```

### Configuration

#### Kconfig

Definition file: `Kconfig`


| Config | Type | Description | Default |
|---|---|---|---|
| `CONFIG_ZMK_TRACKBALL_SHIFT_ROTATION_SAMPLE_TIME_MS` | int | Sample time [ms] for the rotation behavior window. | 3000 |
---

## Fixed Angle Selection

### Overview

Fixed Angle Selection supports setting the device angle directly via behavior key presses. 

### Usage

### Fixed Angle Command Defines

The Trackball Shift Fixed Angle command defines are provided through the [`dt-bindings/zmk/trackball_shift_rotation.h`](https://github.com/karbou12/zmk-trackball-shift/blob/main/include/dt-bindings/zmk/trackball_shift_rotation.h) header, which is added at the top of the keymap file:

```c
#include <dt-bindings/zmk/trackball_shift_rotation.h>
```

This will allow you to reference the actions defined in this header such as `TB_SET_CMD`.

Here is a table describing the command for each define:

| Define | Action |
|---|---|
| `TB_SET_CMD` | Sets the XY input coordinate rotation to the angle specified in parameter #2. |
| `TB_0_DEG` | Sets the XY input coordinate rotation to 0 [deg]. |
| `TB_45_DEG` | Sets the XY input coordinate rotation to 45 [deg]. |
| `TB_90_DEG` | Sets the XY input coordinate rotation to 90 [deg]. |
| `TB_135_DEG` | Sets the XY input coordinate rotation to 135 [deg]. |
| `TB_180_DEG` | Sets the XY input coordinate rotation to 180 [deg]. |
| `TB_225_DEG` | Sets the XY input coordinate rotation to 225 [deg]. |
| `TB_270_DEG` | Sets the XY input coordinate rotation to 270 [deg]. |
| `TB_315_DEG` | Sets the XY input coordinate rotation to 315 [deg]. |

### Behavior Bindings

- Reference: `&tbr`
- Parameter #1: The trackball shift set angle command define, e.g. `TB_SET_CMD`.
- Parameter #2: Device angle [deg] for trackball shift.
    - Only applies to `TB_SET_CMD`.
    - If non-zero, this value must be a multiple of `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG` and a divisor of 360.

### Examples

```c
#include <behaviors/trackball_shift_rotation.dtsi>

/ {
    keymap {
        compatible = "zmk,keymap";
        trackball_shift_layer {
            bindings = <
                &tbr TB_0_DEG     &tbr TB_315_DEG
                &tbr TB_SET_CMD 5 &tbr TB_SET_CMD 10
            >;
        };
    };
};
```

## Direction Detection

### Overview

Detects the physical orientation of the trackball device by rolling the ball from bottom to top while on a specific detection layer or holding a behavior key. 

In detection mode, the direction is determined if the trackball's movement distance exceeds the threshold (`CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_DISTANCE_THRESHOLD`) within the sampling window (`CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_SAMPLE_TIME_MS`). If the threshold is not exceeded within this time, the accumulated distance resets to zero.

While in detection mode, mouse cursor movement is suppressed. Once a direction is successfully detected, the detection process will not execute again until re-entering detection mode.

The orientation resets when the device disconnects.

### Input Processor Usage

When used, the trackball shift direction detection input processor takes no parameters. It **must be used on a specific layer** and **NOT on the default layer**. 

When configuring multiple input processors, the trackball shift direction detection input processor **must be placed after any scaler input processors** in the sequence. Transform input processors must not be used alongside this module.

```c
&zip_trackball_direction_detection
```

#### Example

```c
#include <input_processors/trackball_shift.dtsi>
#include <input_processors/trackball_direction_detection.dtsi>

/ {
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        status = "okay";
        device = <&trackball>;

        // NOT used in the default layer
        input-processors = <
            &zip_trackball_shift
            &zip_mouse_gesture
        >;

        // Must be placed AFTER the scaler in the sequence
        direction_detection {
            layers = <2>;
            input-processors = <
                &zip_xy_scaler 1 3
                &zip_trackball_direction_detection
            >;
        };
    };
};
```

#### Pre-Defined Instances

One pre-defined instance of the trackball shift direction detection input processor is available:

| Reference | Description |
|---|---|
| `&zip_trackball_direction_detection` | Detects the physical orientation of the trackball device by rolling the ball from bottom to top. |

#### User-Defined Instances

Users can define new instances of the trackball shift direction detection input processor if they want to target different codes. 

##### Compatible

The trackball shift input processor uses the `compatible` property `"zmk,input-processor-trackball-direction-detection"`.

##### Standard Properties

- `#input-processor-cells` - Required to be a constant value of `<0>`.

##### User Properties

None.

### Behavior Bindings

- Reference: `&tb_dd`
- Parameters: None

#### Examples

```c
#include <behaviors/trackball_direction_detection.dtsi>

/ {
    keymap {
        compatible = "zmk,keymap";
        trackball_shift_layer {
            bindings = <
                &tb_dd
            >;
        };
    };
};
```

### Configuration

#### Kconfig

Definition file: `Kconfig`



| Config | Type | Description | Default |
|---|---|---|---|
| `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_DISTANCE_THRESHOLD` | int | Distance threshold for trackball shift direction detection. | 800 |
| `CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_SAMPLE_TIME_MS` | int | Sample time [ms] for trackball shift direction detection. | 100 |

#### Devicetree

None.
