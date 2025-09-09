/*********************************************************************
 Based on example code by Adafruit
 
 MIT license, check LICENSE for more information
 Copyright (c) 2025 Twan Kneppers
 All text above must be included in any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"

// Digital pin designation
// Numbers in this array are GPIO pins
const int buttonPins[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

const int encoderOneChannelA = 18;
const int encoderOneChannelB = 19;
const int encoderOneBtn = 16;

const int encoderTwoChannelA = 20;
const int encoderTwoChannelB = 21;
const int encoderTwoBtn = 17;

const int keymapSwitcherPin = encoderOneBtn;

bool keymapSwitcherUsed = false;
int keymapIndex = 0;
// For list of controls check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
// [0] mouse button, [1] mouse delta, [2] mouse X direction, [3] mouse Y direction, [4] scroll delta [5] scroll vertical, [6] scroll horizontal, [7-12] 6 x keyboard keys, [13] consumer control
// value should be 0 if not applicable
// mouse direction = 0 if no movement
// mouse button and mouse movement can't happen in the same macro
const int keymap[3][12][14] = {
  {{0, 0, 0, 0, 0, 0, 0, HID_KEY_F13, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F14, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F15, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F16, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F17, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F18, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F19, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F20, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F21, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F22, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F23, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_F24, 0, 0, 0, 0, 0, 0}},

  {{0, 0, 0, 0, 0, 0, 0, HID_KEY_7, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_8, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_9, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_4, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_5, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_6, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_3, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_PERIOD, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_ENTER, 0, 0, 0, 0, 0, 0}},

  {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_SCAN_NEXT_TRACK},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_PLAY_PAUSE},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_X, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_C, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_V, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_Z, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_CONTROL_LEFT, HID_KEY_S, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_ALT_LEFT, HID_KEY_TAB, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, HID_USAGE_CONSUMER_AL_CALCULATOR},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_PRINT_SCREEN, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, HID_KEY_DELETE, 0, 0, 0, 0, 0, 0}}
  };

int mouseButtonPin = -1;
int pressedKeyPin = -1;
int consumerControlPinIndex = -1;

bool hasMouseButton = false;
bool hasKeyboardKey = false;
bool consumerKeyUsed = false;

bool encoderTwoBtnUsed = false;

// 0 clockwise, 1 counter clockwise
int encoderOneDirection = -1;
int encoderTwoDirection = -1;

bool encoderOneSent = false;
bool encoderTwoSent = false;

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

  pinMode(encoderOneChannelA, INPUT_PULLUP);
  pinMode(encoderOneChannelB, INPUT_PULLUP);
  pinMode(encoderOneBtn, INPUT_PULLUP);
  pinMode(encoderTwoChannelA, INPUT_PULLUP);
  pinMode(encoderTwoChannelB, INPUT_PULLUP);
  pinMode(encoderTwoBtn, INPUT_PULLUP);
  pinMode(keymapSwitcherPin, INPUT_PULLUP);
}

void keymapSwitcher() {
  if (digitalRead(keymapSwitcherPin) == LOW && !keymapSwitcherUsed) {
    keymapSwitcherUsed = true;
    if (keymapIndex == 2) { 
      keymapIndex = 0;
    }
    else {
      keymapIndex++;
    }
  }
  else if (digitalRead(keymapSwitcherPin) == HIGH && keymapSwitcherUsed) {
    keymapSwitcherUsed = false;
    delay(10);
  }
}

void keys() {
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
    if (mouseButtonPin != -1 && hasMouseButton) {
      if (digitalRead(mouseButtonPin) == HIGH) {
        usb_hid.mouseButtonRelease(RID_MOUSE);
        hasMouseButton = false;
        mouseButtonPin = -1;
        delay(10);
      }
    }

    // Keyboard key release
    if (pressedKeyPin != -1 && hasKeyboardKey) {
      if (digitalRead(pressedKeyPin) == HIGH) {
        usb_hid.keyboardRelease(RID_KEYBOARD);
        pressedKeyPin = -1;
        hasKeyboardKey = false;
        delay(10);
      }
    }

    // Consumer key release
    if (consumerControlPinIndex != -1 && consumerKeyUsed) {
      if (digitalRead(buttonPins[consumerControlPinIndex]) == HIGH) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_KEY_NONE);
        consumerKeyUsed = false;
        consumerControlPinIndex = -1;
        delay(10);
      }
    }

    if (anyKeyPressed == true) {
      for (int pinIndex = 0; pinIndex < 12; pinIndex++) {

        // Mouse control
        if (digitalRead(buttonPins[pinIndex]) == LOW) {    
          int mouseDelta = keymap[keymapIndex][pinIndex][1];

          if (mouseDelta > 0) {
            int deltaX = keymap[keymapIndex][pinIndex][2] * mouseDelta;
            int deltaY = keymap[keymapIndex][pinIndex][3] * mouseDelta;
            //Calculated from top left of display
            usb_hid.mouseMove(RID_MOUSE, deltaX, deltaY);
            delay(10);
          }
          else if (keymap[keymapIndex][pinIndex][0] != 0 && !hasMouseButton) {
            usb_hid.mouseButtonPress(RID_MOUSE, keymap[keymapIndex][pinIndex][0]);
            hasMouseButton = true;
            mouseButtonPin = pinIndex;
            delay(10);
          }
          else if (keymap[keymapIndex][pinIndex][4] != 0) {
            int scrollDelta = keymap[keymapIndex][pinIndex][4];
            int verticalScrollDirection = keymap[keymapIndex][pinIndex][5];
            int horizontalScrollDirection = keymap[keymapIndex][pinIndex][6];

            int verticalScroll = scrollDelta * verticalScrollDirection;
            int horizontalScroll = scrollDelta * horizontalScrollDirection;

            usb_hid.mouseScroll(RID_MOUSE, verticalScroll, horizontalScroll);
            delay(10);
          }
        }

        // Keyboard control
        if (digitalRead(buttonPins[pinIndex]) == LOW && keymap[keymapIndex][pinIndex][7] != 0 && !hasKeyboardKey) {
          // A maximum of 6 keys can be sent in 1 report
          // Adding to the keycode array adds a key that is being 'pressed' in order from 0 to 5
          uint8_t keycode[6] = {0};
        
          for (int codeIndex = 0; codeIndex < 6; codeIndex++) {
            keycode[codeIndex] = keymap[keymapIndex][pinIndex][(codeIndex + 7)];
          }

          //I don't know what the 0 means
          usb_hid.keyboardReport(RID_KEYBOARD, 0, keycode);
          pressedKeyPin = buttonPins[pinIndex];
          hasKeyboardKey = true;
          delay(10);
        } 

        // Consumer control
        if (digitalRead(buttonPins[pinIndex]) == LOW && !consumerKeyUsed) {
          usb_hid.sendReport16(RID_CONSUMER_CONTROL, keymap[keymapIndex][pinIndex][13]);
          consumerKeyUsed = true;
          consumerControlPinIndex = pinIndex;
          delay (10);
        } 
      }  
    }
  }
}

void encoderOne() {
  // Rotation
  if (encoderOneDirection == -1 && digitalRead(encoderOneChannelA) == LOW && digitalRead(encoderOneChannelB) == HIGH) {
    encoderOneDirection = 0;
  }
  else if (encoderOneDirection == -1 && digitalRead(encoderOneChannelA) == HIGH && digitalRead(encoderOneChannelB) == LOW) {
    encoderOneDirection = 1;
  }
  else if (encoderOneDirection != -1 && usb_hid.ready() && digitalRead(encoderOneChannelA) == HIGH && digitalRead(encoderOneChannelB) == HIGH) {
    if (!encoderOneSent) {
      if (encoderOneDirection == 0) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT);
        delay(10);
      }
      else {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT);
        delay(10);
      }
      encoderOneSent = true;
    }
    else {
      usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_KEY_NONE);
      encoderOneDirection = -1;
      encoderOneSent = false;
      delay(10);
    }
  }
}

void encoderTwo() {
  // Rotation
  if (encoderTwoDirection == -1 && digitalRead(encoderTwoChannelA) == LOW && digitalRead(encoderTwoChannelB) == HIGH) {
    encoderTwoDirection = 0;
  }
  else if (encoderTwoDirection == -1 && digitalRead(encoderTwoChannelA) == HIGH && digitalRead(encoderTwoChannelB) == LOW) {
    encoderTwoDirection = 1;
  }
  else if (encoderTwoDirection != -1 && usb_hid.ready() && digitalRead(encoderTwoChannelA) == HIGH && digitalRead(encoderTwoChannelB) == HIGH) {
    if (!encoderTwoSent) {
      if (encoderTwoDirection == 0) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_VOLUME_INCREMENT);
        delay(10);
      }
      else {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_VOLUME_DECREMENT);
        delay(10);
      }
      encoderTwoSent = true;
    }
    else {
      usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_KEY_NONE);
      encoderTwoDirection = -1;
      encoderTwoSent = false;
      delay(10);
    }
  }

  // Button
  if (usb_hid.ready()) {
    if (encoderTwoBtnUsed && digitalRead(encoderTwoBtn) == HIGH) {
      usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_KEY_NONE);
      encoderTwoBtnUsed = false;   
      delay(10);
    }
    else if (!encoderTwoBtnUsed && digitalRead(encoderTwoBtn) == LOW) {
      usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_MUTE);
      encoderTwoBtnUsed = true;
      delay (10);
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

  // Switches keymap on button press
  keymapSwitcher();

  // Encoder one
  encoderOne();

  // Encoder two
  encoderTwo();

  keys();
}
