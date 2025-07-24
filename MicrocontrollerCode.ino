/*********************************************************************
 Based on example code by Adafruit
 
 MIT license, check LICENSE for more information
 Copyright (c) 2019 Twan Kneppers
 All text above must be included in any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"

const int buttonPin1 = 2;
const int buttonPin2 = 3;

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

  // Set up buttons
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
}

void process_hid() {
  // Remote wakeup
  if (TinyUSBDevice.suspended() && digitalRead(buttonPin1) == LOW) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  /*------------- Mouse -------------*/
  /*if (usb_hid.ready() && digitalRead(buttonPin1) == LOW) {
    int8_t const delta = 5;
    usb_hid.mouseMove(RID_MOUSE, delta, delta); // right + down

    // delay a bit before attempt to send keyboard report
    delay(10);
  }*/

  /*------------- Keyboard -------------*/
  if (usb_hid.ready()) {
    // use to send key release report
    static bool has_key = false;

    if (digitalRead(buttonPin1) == LOW) {
      // A maximum of six (6) keys can be sent in one (1) report
      // Adding to the keycode array adds a key that is being 'pressed' in order from 0 to 5
      uint8_t keycode[6] = {0};
      keycode[0] = HID_KEY_SHIFT_LEFT;
      keycode[1] = HID_KEY_A;

      //I don't know what the 0 means
      usb_hid.keyboardReport(RID_KEYBOARD, 0, keycode);

      has_key = true;
    } else {
      // send empty key report if previously has key pressed
      if (has_key) usb_hid.keyboardRelease(RID_KEYBOARD);
      has_key = false;
    }

    // delay a bit before attempt to send consumer report
    delay(10);
  }

  /*------------- Consumer Control -------------*/
  if (usb_hid.ready()) {
    // For list of control check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h

    // use to send consumer release report
    static bool has_consumer_key = false;

    if (digitalRead(buttonPin2) == LOW) {
      usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_PLAY_PAUSE);
      has_consumer_key = true;
    } else {
      // release the consume key by sending zero (0x0000)
      if (has_consumer_key) usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
      has_consumer_key = false;
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