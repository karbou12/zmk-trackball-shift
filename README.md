# ZMK Module: zmk-feature-trackball_shift

A ZMK input processor module that detect a direction of trackball device, and rotate xy input.
The module name is inspired from OctaShift of Nape Pro.

Note: Once I created a similaor feature for Nape Origin by modifying pmw3610 driver.
I extracted the feature as a module and improve it.

## Features

- Rotates xy input coordinate by the default angle that defined in configuration file.

## Configuration

- input-processors
  - trackball_shift
    - direction-angle-degree
      - angle of each direction for trackball shift. the range is from 3 to 45.
    - default-device-angle-degree
      - default device angle for trackball shift. it should be divided by direcion-angle-degree if it is not zero.
 
