# ZMK Module: zmk-feature-trackball_shift

A ZMK input processor module that detect a direction of trackball device, and rotate xy input.
The module name is inspired from OctaShift of Nape Pro.

Note: Once I created a similaor feature for Nape Origin by modifying pmw3610 driver.
I extracted the feature as a module and improve it.

## Features

- Rotates xy input coordinate by the default angle that defined in configuration file.
- Supports CW/CCW rotation of the coordinate by key press and rotary encoder. A rotation angle is configurable.
- Detects a direction of trackball device by rolling a trackball from down to up on a detection layer. It is reset on disconnect of the device.

## Configuration

- input-processors
  - trackball_shift
    - direction-angle-degree
      - angle of each direction for trackball shift. the range is from 3 to 45.
      - default value is 45.
    - default-device-angle-degree
      - default device angle for trackball shift. it should be divided by direcion-angle-degree if it is not zero.
      - default value is 45.
    - rotation-sample-time-ms
      - sample time for rotation by a rotary encoder.
      - default value is 3000.
 
  - trackball_direction_detection
    - distance-threshold
      - threshold of distance for direction detection of trackball shift.
      - devault value is 800.
    - detection-sample-time-ms
      - sample time [ms] for direction detection of trackball shift.
      - default value is 100.

- behavior
  - trackball_shift_rotation
    - step-angle-degree
      - the step of the rotation angle of the device, the range is from 3 to 45.
      - it considers multible input by a rotary encoder.
        if the step is less than direction-angle-degree of trackball_shift input-processors,
        it rotate angle with direction-angle-degree when total step is greater than direction-angle-degree within rotation-sample-time-ms.
      - default value is 45.
    - param1
      - TB_ROT_CW : rotate clockwise
      - TB_ROT_CCW : rotate counter-clockwise
