Code and files we wrote for a school research project.

This project uses a Raspberry Pi Pico 2 family microcontroller to make a reprogrammable macropad. The microncontroller code is based on code from the Adafruit TinyUSB Library for Arduino (https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/master/examples/HID/hid_composite/hid_composite.ino).

Microcontroller setup guide:
-

  1. Connect the microcontroller to your computer as a mass storage device by holding the button labelled "Bootsel" while plugging the USB-connector into your computer, it should show up as "RP2350"
  2. Download the latest release of the microcontroller code 
  3. Extract the downloaded "Microcontroller.code.zip" file 
  4. Move the folder labelled "Microcontroller code" over to the root of the Pico. Be sure to move the entire folder, not just its contents

If done properly, the Pico should disconnect automatically after moving over the folder.

PCB:
-
PCB designed with KiCad 9.0 and should work with PCB ordering services such as PCBWay.


Standard keymaps:
-

Encoders:
 1. Press: switch keymap, Turn: change screen brightness
 2. Press: mute volume, Turn: change volume

Keymap 1:  Sends signals to the companion app

  1. F13
  2. F14
  3. F15
  4. F16
  5. F17
  6. F18
  7. F19
  8. F20
  9. F21
  10. F22
  11. F23
  12. F24

Keymap 2: Mouse

 1. Left click
 2. Middle click
 3. Right click
 4. Scroll up
 5. Move mouse up
 6. Scroll down
 7. Move mouse left
 8. Scroll lock
 9. Move mouse right
 10. Scroll left
 11. Move mouse down
 12. Scroll right

Keymap 3: Various shortcuts

 1. Previous track
 2. Next track
 3. Play/pause
 4. Ctrl + x
 5. Ctrl + c
 6. Ctrl + v
 7. Ctrl + z
 8. Ctrl + s
 9. Alt + tab
 10. Calculator
 11. Screenshot
 12. Delete

Components:
- 
Off the shelf:
 - PEC12R-4217F-S0024 rotary encoder
 - Kaihl hotswap sockets for cherry mx
 - Cherry MX keyswitches
 - Raspberry Pi Pico 2 + socket
 - Keycaps for cherry mx (optional)

Custom:
 - PCB
 - 3d printed case
 - 3d printed knobs
 - 3d printed keycaps
