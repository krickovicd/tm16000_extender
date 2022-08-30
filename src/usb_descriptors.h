/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#ifdef __cplusplus
extern "C"
{
#endif

// Joystick Report Descriptor Template
// with 32 buttons, 4 joysticks and 2 hat/dpad with following layout
// | Button Map (4 bytes) | hat/DPAD (1 byte) | X | Y | Z | Slider (2 byte each) |
#define TUD_HID_REPORT_DESC_JOYSTICK(...)                                                      \
  HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                                      \
      HID_USAGE(HID_USAGE_DESKTOP_JOYSTICK),                                                   \
      HID_COLLECTION(HID_COLLECTION_APPLICATION), /* Report ID if any */                       \
      __VA_ARGS__                                 /* 32 bit Button Map */                      \
      HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),                                                   \
      HID_USAGE_MIN(1),                                                                        \
      HID_USAGE_MAX(32),                                                                       \
      HID_LOGICAL_MIN(0),                                                                      \
      HID_LOGICAL_MAX(1),                                                                      \
      HID_REPORT_COUNT(32),                                                                    \
      HID_REPORT_SIZE(1),                                                                      \
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), /* 8 bit DPad/Hat Button Map  */      \
      HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                                  \
      HID_USAGE(HID_USAGE_DESKTOP_HAT_SWITCH),                                                 \
      HID_LOGICAL_MIN(0),                                                                      \
      HID_LOGICAL_MAX(7),                                                                      \
      HID_PHYSICAL_MIN(0),                                                                     \
      HID_PHYSICAL_MAX_N(315, 2),                                                              \
      HID_REPORT_COUNT(1),                                                                     \
      HID_REPORT_SIZE(4),                                                                      \
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), /* 8 bit DPad/Hat Button Map  */      \
      HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                                  \
      HID_USAGE(HID_USAGE_DESKTOP_HAT_SWITCH),                                                 \
      HID_LOGICAL_MIN(0),                                                                      \
      HID_LOGICAL_MAX(7),                                                                      \
      HID_PHYSICAL_MIN(0),                                                                     \
      HID_PHYSICAL_MAX_N(315, 2),                                                              \
      HID_REPORT_COUNT(1),                                                                     \
      HID_REPORT_SIZE(4),                                                                      \
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), /* 16 bit X, Y (min 0, max 65535 ) */ \
      HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                                  \
      HID_USAGE(HID_USAGE_DESKTOP_X),                                                          \
      HID_USAGE(HID_USAGE_DESKTOP_Y),                                                          \
      HID_USAGE(HID_USAGE_DESKTOP_Z),                                                          \
      HID_USAGE(HID_USAGE_DESKTOP_SLIDER),                                                     \
      HID_LOGICAL_MIN(0x00),                                                                   \
      HID_LOGICAL_MAX_N(0x0000ffff, 3),                                                        \
      HID_REPORT_COUNT(4),                                                                     \
      HID_REPORT_SIZE(16),                                                                     \
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),                                       \
      HID_COLLECTION_END

  enum
  {
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_MOUSE,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_GAMEPAD,
    REPORT_ID_COUNT
  };

#define JOYSTICK_DEFAULT_REPORT_ID 0x03
#define JOYSTICK_DEFAULT_BUTTON_COUNT 32
#define JOYSTICK_DEFAULT_AXIS_MINIMUM 0
#define JOYSTICK_DEFAULT_AXIS_MAXIMUM 65535
#define JOYSTICK_DEFAULT_SIMULATOR_MINIMUM 0
#define JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM 65535
#define JOYSTICK_DEFAULT_HATSWITCH_COUNT 2
#define JOYSTICK_HATSWITCH_COUNT_MAXIMUM 2
#define JOYSTICK_HATSWITCH_RELEASE -1
#define JOYSTICK_TYPE_JOYSTICK 0x04
#define JOYSTICK_TYPE_GAMEPAD 0x05
#define JOYSTICK_TYPE_MULTI_AXIS 0x08

#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

  typedef struct 
  {
    // Joystick State
    uint32_t _xAxis;
    uint32_t _yAxis;
    uint32_t _zAxis;
    uint32_t _slider;
    uint16_t _hatSwitchValues[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
    uint8_t *_buttonValues;

    // Joystick Settings
    bool _autoSendState;
    uint8_t _buttonCount;
    uint8_t _buttonValuesArraySize;
    uint8_t _hatSwitchCount;
  } tm_joystick_t;

  typedef struct TU_ATTR_PACKED
  {
    uint8_t buttons[4];
    uint8_t hat[1];
    uint8_t x[2];
    uint8_t y[2];
    uint8_t z[2];
    uint8_t s[2];

  } tm_joystick_report;

  static tm_joystick_t tm_joystick;

  uint32_t buildAndSet16BitValue(uint32_t value, uint32_t valueMinimum, uint32_t valueMaximum, uint32_t actualMinimum, uint32_t actualMaximum, uint8_t dataLocation[]);
  uint32_t buildAndSetAxisValue(uint32_t axisValue, uint32_t axisMinimum, uint32_t axisMaximum, uint8_t dataLocation[]);
  uint32_t buildAndSetSimulationValue(uint32_t value, uint32_t valueMinimum, uint32_t valueMaximum, uint8_t dataLocation[]);

  void tm_joystick_setup();

  void begin(bool initAutoSendState);
  void end();

  // Set Axis Values
  void tm_joystick_setXAxis(uint32_t value);
  void tm_joystick_setYAxis(uint32_t value);
  void tm_joystick_setZAxis(uint32_t value);
  void tm_joystick_setSliderAxis(uint32_t value);

  void tm_joystick_setButton(uint8_t button, uint8_t value);
  void tm_joystick_pressButton(uint8_t button);
  void tm_joystick_releaseButton(uint8_t button);

  void tm_joystick_setHatSwitch(uint8_t hatSwitch, uint16_t value);

  void tm_joystick_fill_report(tm_joystick_report *report);

#ifdef __cplusplus
}
#endif

#endif /* USB_DESCRIPTORS_H_ */
