/*********************************************************************
 Based on example code by Adafruit
 
 MIT license, check LICENSE for more information
 Copyright (c) 2025 Twan Kneppers
 All text above must be included in any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"

// Numbers in this array are GPIO pins
const int buttonPins[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

// For list of controls check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
// [0] mouse button, [1] mouse delta, [2] mouse X direction, [3] mouse Y direction, [4] scroll delta [5] scroll vertical, [6] scroll horizontal, [7-12] 6 x keyboard keys, [13] consumer control
// value should be 0 if not applicable
// mouse direction = 0 if no movement
// mouse button and mouse movement can't happen in the same macro
const int keymap[12][14] = {
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_C, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_V, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_PLAY_PAUSE},
  {MOUSE_BUTTON_LEFT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_ALT_LEFT, HID_KEY_DELETE, 0, 0, 0, 0},
  {0, 0, 0, 0, 25, -1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_AL_CALCULATOR},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_MUTE}
};

int mouseButtonPin = -1;
int pressedKeyPin = -1;
int consumerControlPin = -1;

// Report ID
enum {
  RID_KEYBOARD = 1,
  RID_MOUSE,
  RID_CONSUMER_CONTROL,
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(RID_MOUSE)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL))
};

// USB HID object.
Adafruit_USBD_HID usb_hid;

// the setup function runs once when you press reset or power the board
void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  Serial.begin(115200);

  // Set up HID
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("Macropad");
  usb_hid.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  // Sets up button pins
  for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
    pinMode(buttonPins[pinIndex], INPUT_PULLUP);
  }
}

void process_hid() {

  // Remote wakeup
  int digitalPinSum = 0;
  bool anyKeyPressed = false;

  for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
    digitalPinSum = digitalPinSum + digitalRead(buttonPins[pinIndex]);
  }

  if (digitalPinSum < 12) {
    anyKeyPressed = true;
  }

  if (TinyUSBDevice.suspended() && anyKeyPressed == true) {
    TinyUSBDevice.remoteWakeup();
  }

  if (usb_hid.ready()) {

    // Mouse button release
    static bool hasMouseButton = false;

    if (mouseButtonPin != -1 && hasMouseButton) {
      if (digitalRead(mouseButtonPin) == HIGH) {
        usb_hid.mouseButtonRelease(RID_MOUSE);
        hasMouseButton = false;
        mouseButtonPin = -1;
        delay(10);
      }
    }

    // Keyboard key release
    static bool hasKeyboardKey = false;

    if (pressedKeyPin != -1 && hasKeyboardKey) {
      if (digitalRead(pressedKeyPin) == HIGH) {
        usb_hid.keyboardRelease(RID_KEYBOARD);
        pressedKeyPin = -1;
        hasKeyboardKey = false;
        delay(10);
      }
    }

    // Consumer key release
    static bool hasConsumerKey = false;
    static bool consumerKeyUsed = false;

    for (int pinIndex = 0; pinIndex < 12; pinIndex++) {

      // Mouse control
      if (digitalRead(buttonPins[pinIndex]) == LOW) {    
        int mouseDelta = keymap[pinIndex][1];

        if (mouseDelta > 0) {
          int deltaX = keymap[pinIndex][2] * mouseDelta;
          int deltaY = keymap[pinIndex][3] * mouseDelta;
          //Calculated from top left of display
          usb_hid.mouseMove(RID_MOUSE, deltaX, deltaY);
          delay(10);
        }
        else if (keymap[pinIndex][0] != 0 && !hasMouseButton) {
          usb_hid.mouseButtonPress(RID_MOUSE, keymap[pinIndex][0]);
          hasMouseButton = true;
          mouseButtonPin = pinIndex;
          delay(10);
        }
        else if (keymap[pinIndex][4] != 0) {
          int scrollDelta = keymap[pinIndex][4];
          int verticalScrollDirection = keymap[pinIndex][5];
          int horizontalScrollDirection = keymap[pinIndex][6];

          int verticalScroll = scrollDelta * verticalScrollDirection;
          int horizontalScroll = scrollDelta * horizontalScrollDirection;

          usb_hid.mouseScroll(RID_MOUSE, verticalScroll, horizontalScroll);
        }
      }

      // Keyboard control
      if (digitalRead(buttonPins[pinIndex]) == LOW && keymap[pinIndex][4] != 0 && !hasKeyboardKey) {
        // A maximum of 6 keys can be sent in 1 report
        // Adding to the keycode array adds a key that is being 'pressed' in order from 0 to 5
        uint8_t keycode[6] = {0};
        
        //Turning this into a for loop doesn't work
        keycode[0] = keymap[pinIndex][7];
        keycode[1] = keymap[pinIndex][8];
        keycode[2] = keymap[pinIndex][9];
        keycode[3] = keymap[pinIndex][10];
        keycode[4] = keymap[pinIndex][11];
        keycode[5] = keymap[pinIndex][12];

        //I don't know what the 0 means
        usb_hid.keyboardReport(RID_KEYBOARD, 0, keycode);
        pressedKeyPin = buttonPins[pinIndex];
        hasKeyboardKey = true;
        delay(10);
      } 

      // Consumer control
      if (digitalRead(buttonPins[pinIndex]) == LOW && !consumerKeyUsed) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, keymap[pinIndex][13]);
        hasConsumerKey = true;
        consumerKeyUsed = true;
        consumerControlPin = pinIndex;
      } 
      else {
        // release the consumer key by sending zero (0x0000)
        if (hasConsumerKey) {
          usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
          hasConsumerKey = false;
        }
      }

      if (consumerControlPin != -1) {
        if (digitalRead(buttonPins[consumerControlPin]) == HIGH) {
          consumerKeyUsed = false;
          consumerControlPin = -1;
        }
      }
    }
  }
}

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // poll gpio once each 10 ms
  static uint32_t ms = 0;
  if (millis() - ms > 10) {
    ms = millis();
    process_hid();
  }
}
