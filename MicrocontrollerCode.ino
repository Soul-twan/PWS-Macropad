/*********************************************************************
 Based on example code by Adafruit
 
 MIT license, check LICENSE for more information
 Copyright (c) 2025 Twan Kneppers
 All text above must be included in any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"

// numbers in this array are GPIO pins
const int buttonPins[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

// For list of controls check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
// [0] mouse button, [1] mouse delta, [2] mouse X direction, [3] mouse Y direction, [4-9] 6 x keyboard keys, [10] consumer control
// value should be 0 if not applicable
// mouse direction = 0 if no movement
// mouse button and mouse movement can't happen in the same macro
const int keymap[12][11] = {
  {0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_C, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_V, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_PLAY_PAUSE},
  {MOUSE_BUTTON_LEFT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, -1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 5, 0, -1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_ALT_LEFT, HID_KEY_DELETE, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_AL_LOCAL_BROWSER},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_AL_CALCULATOR},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_MUTE}
};

int mouseButtonPin = -1;
int pressedKeyPin = -1;
int consumerControlPin = -1;
int keyboardKey = 4;
int keycodeIndex = 0;

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

  for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
    pinMode(buttonPins[pinIndex], INPUT_PULLUP);
  }
}

void process_hid() {
  int digitalPinSum = 0;
  bool anyKeyPressed = false;

  for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
    digitalPinSum = digitalPinSum + digitalRead(buttonPins[pinIndex]);
  }

  if (digitalPinSum < 12) {
    anyKeyPressed = true;
  }

  // Remote wakeup
  if (TinyUSBDevice.suspended() && anyKeyPressed == true) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  /*------------- Mouse -------------*/
  if (usb_hid.ready()) {
    static bool hasMouseButton = false;

    if (mouseButtonPin != -1 && hasMouseButton) {
      if (digitalRead(mouseButtonPin) == HIGH) {
        usb_hid.mouseButtonRelease(RID_MOUSE);
        hasMouseButton = false;
        mouseButtonPin = -1;
        delay(10);
      }
    }

    for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
      if (digitalRead(buttonPins[pinIndex]) == LOW) {    
        int delta = keymap[pinIndex][1];

        if (delta > 0) {
          int deltaX = keymap[pinIndex][2] * delta;
          int deltaY = keymap[pinIndex][3] * delta;
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
      }
    }
  }

  /*------------- Keyboard -------------*/
  if (usb_hid.ready()) {
    // use to send key release report
    static bool has_key = false;

    if (pressedKeyPin != -1 && has_key) {
      if (digitalRead(pressedKeyPin) == HIGH) {
        usb_hid.keyboardRelease(RID_KEYBOARD);
        pressedKeyPin = -1;
        has_key = false;
        delay(10);
      }
    }

    for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
      if (digitalRead(buttonPins[pinIndex]) == LOW && keymap[pinIndex][4] != 0 && !has_key) {
        // A maximum of six (6) keys can be sent in one (1) report
        // Adding to the keycode array adds a key that is being 'pressed' in order from 0 to 5
        uint8_t keycode[6] = {0};
        
        //Turning this into a for loop doesn't work
        keycode[0] = keymap[pinIndex][4];
        keycode[1] = keymap[pinIndex][5];
        keycode[2] = keymap[pinIndex][6];
        keycode[3] = keymap[pinIndex][7];
        keycode[4] = keymap[pinIndex][8];
        keycode[5] = keymap[pinIndex][9];

        /*for (keyboardKey = 4; keyboardKey < 10; keyboardKey++) {
          for (keycodeIndex = 0; keycodeIndex < 6; keycodeIndex++) {
            keycode[keycodeIndex] = keymap[pinIndex][keyboardKey];
          }
        }*/

        //I don't know what the 0 means
        usb_hid.keyboardReport(RID_KEYBOARD, 0, keycode);
        pressedKeyPin = buttonPins[pinIndex];
        has_key = true;
        delay(10);
      } 
     else {
      }
    }
  }

  /*------------- Consumer Control -------------*/
  if (usb_hid.ready()) {
    // used to send consumer release report and makes sure key isn't triggered everytime process runs while button is held down
    static bool has_consumer_key = false;
    static bool consumerKeyUsed = false;

    for (int pinIndex = 0; pinIndex < 12; pinIndex++) {
      if (digitalRead(buttonPins[pinIndex]) == LOW && !consumerKeyUsed) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, keymap[pinIndex][10]);
        has_consumer_key = true;
        consumerKeyUsed = true;
        consumerControlPin = pinIndex;
      } 
      else {
        // release the consumer key by sending zero (0x0000)
        if (has_consumer_key) {
          usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
          has_consumer_key = false;
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
