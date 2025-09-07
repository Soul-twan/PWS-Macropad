Code and files we wrote for a school research project.

This project uses a Raspberry Pi Pico 2 family microcontroller to make a reprogrammable macropad. The microncontroller code is based on code from the Adafruit TinyUSB Library for Arduino (https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/master/examples/HID/hid_composite/hid_composite.ino).

Microcontroller setup guide:
  1. Connect the microcontroller to your computer as a mass storage device by holding the button labelled "Bootsel" while plugging the USB-connector into your computer, it should show up as "RP2350"
  2. Download the latest release of the microcontroller code 
  3. Extract the downloaded "Microcontroller.code.zip" file 
  4. Move the folder labelled "Microcontroller code" over to the root of the Pico. Be sure to move the entire folder, not just its contents

If done properly, the Pico should disconnect automatically after moving over the folder.
