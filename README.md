# ZMK Module: zmk-feature-trackball_shift

A ZMK input processor module that detect a direction of trackball device, and rotate xy input.
The module name is inspired from OctaShift of Nape Pro.

Note: Once I created a similaor feature for Nape Origin by modifying pmw3610 driver.
I extracted the feature as a module and improve it.

## Features

- Rotates xy input coordinate by the default angle that defined in configuration file.
- Supports CW/CCW rotation of the coordinate by key press and rotary encoder. A rotation angle is configurable.
- Detects a direction of trackball device by rolling a trackball from down to up on a detection layer, or on holding a behavior key. It is reset on disconnect of the device.

## Configuration

### trackball-shift

#### Kconfig

- CONFIG_ZMK_TRACKBALL_SHIFT_DIRECTION_ANGLE_DEG
  - Angle [deg] of each direction for trackball shift.
  - It must be a dvisor of 360.
  - It must be between 3 and 45.
  - Default value is 45.

- CONFIG_ZMK_TRACKBALL_SHIFT_ROTATION_SAMPLE_TIME_MS
  - Sample time [ms] for rotation behavior.
  - Used for handling multiple inputs from a rotary encoder.
  - If the encoder's step angle is smaller than direction_angle_deg,
    steps are accumulated within the sample time.
  - Once the accumulated angle exceeds direction_angle_deg, the system triggers a rotation of direction_angle_deg.
  - If the threshold is not exceeded within this time, the accumulated steps are reset.
  - Default value is 3000.

####  input-processors

- `xmk,input-processor-trackball-shift`
  - properties in dtsi
    - default-device-angle-degree
      - Default device angle [deg] for trackball shift.
      - If non-zero, this value must be a multiple of the direction_angle_deg, and a divisor of 360.
      - Default value is 0.
 
### trackball-shift-rotation

#### behavior

- `zmk,behavior-trackball-shift-rotation`
  - properties in dtsi
    - step-angle-degree
      - the step of the rotation angle of the device, the range is from 3 to 45.
      - it considers multible input by a rotary encoder.
        if the step is less than direction-angle-degree of trackball_shift input-processors,
        it rotate angle with direction-angle-degree when total step is greater than direction-angle-degree within rotation-sample-time-ms.
      - default value is 45.
  - param1 in keymap
    - TB_ROT_CW : rotate clockwise
    - TB_ROT_CCW : rotate counter-clockwise

### trackball-direction-detection

#### Kconfig

- CONFIG_ZMK_TRACKBALL_SHIFT_DISTANCE_THRESHOLD
  - threshold of distance for direction detection of trackball shift.
  - devault value is 800.
- CONFIG_ZMK_TRACKBALL_SHIFT_DETECTION_SAMPLE_TIME_MS
  - sample time [ms] for direction detection of trackball shift.
  - default value is 100.

#### input_processors

- `zmk,input-processor-trackball-direction-detection`
  - properties in dtsi
    - distance-threshold
      - the purpose is same with CONFIG_ZMK_TRACKBALL_SHIFT_DISTANCE_THRESHOLD.
      - if both of properties and conf are set, properties is used.
    - detection-sample-time-ms
      - the purpose is same with CONFIG_ZMK_TRACKBALL_SHIFT_DETECTION_SAMPLE_TIME_MS.
      - if both of properties and conf are set, properties is used.

#### behavior

- `zmk,behavior-trackball-direction-detection`
  - no properties, and no params.

