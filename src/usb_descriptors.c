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
 *
 */

#include "tusb.h"
#include "usb_descriptors.h"
#include <pico/stdlib.h>
#include <stdlib.h>

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                 _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define USB_VID 0xCafe
#define USB_BCD 0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = USB_BCD,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = USB_VID,
        .idProduct = USB_PID,
        .bcdDevice = 0x0100,

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
  return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_JOYSTICK()};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
  (void)instance;
  return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID 0x81

uint8_t const desc_configuration[] =
    {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

        // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
        TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, /*113*/ sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)};

#if TUD_OPT_HIGH_SPEED
// Per USB specs: high speed capable device must report device_qualifier and other_speed_configuration

// other speed configuration
uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
tusb_desc_device_qualifier_t const desc_device_qualifier =
    {
        .bLength = sizeof(tusb_desc_device_qualifier_t),
        .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
        .bcdUSB = USB_BCD,

        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,

        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
        .bNumConfigurations = 0x01,
        .bReserved = 0x00};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const *tud_descriptor_device_qualifier_cb(void)
{
  return (uint8_t const *)&desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
  (void)index; // for multiple configurations

  // other speed config is basically configuration with type = OHER_SPEED_CONFIG
  memcpy(desc_other_speed_config, desc_configuration, CONFIG_TOTAL_LEN);
  desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

  // this example use the same configuration for both high and full speed mode
  return desc_other_speed_config;
}

#endif // highspeed

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index; // for multiple configurations

  // This example use the same configuration for both high and full speed mode
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] =
    {
        (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
        "TinyUSB",                  // 1: Manufacturer
        "TinyUSB Device",           // 2: Product
        "123456",                   // 3: Serials, should use chip ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;

  uint8_t chr_count;

  if (index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }
  else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
      return NULL;

    const char *str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if (chr_count > 31)
      chr_count = 31;

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < chr_count; i++)
    {
      _desc_str[1 + i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

  return _desc_str;
}

//--------------------------------------------------------------------+
// JOYSTICK IMPLEMENTATION
//--------------------------------------------------------------------+

#define JOYSTICK_REPORT_ID_INDEX 7
#define JOYSTICK_AXIS_MINIMUM 0
#define JOYSTICK_AXIS_MAXIMUM 65535
#define JOYSTICK_SIMULATOR_MINIMUM 0
#define JOYSTICK_SIMULATOR_MAXIMUM 65535

#define JOYSTICK_INCLUDE_X_AXIS 0B00000001
#define JOYSTICK_INCLUDE_Y_AXIS 0B00000010
#define JOYSTICK_INCLUDE_Z_AXIS 0B00000100
#define JOYSTICK_INCLUDE_RX_AXIS 0B00001000
#define JOYSTICK_INCLUDE_RY_AXIS 0B00010000
#define JOYSTICK_INCLUDE_RZ_AXIS 0B00100000

#define JOYSTICK_INCLUDE_RUDDER 0B00000001
#define JOYSTICK_INCLUDE_THROTTLE 0B00000010
#define JOYSTICK_INCLUDE_ACCELERATOR 0B00000100
#define JOYSTICK_INCLUDE_BRAKE 0B00001000
#define JOYSTICK_INCLUDE_STEERING 0B00010000

#define TM_REPORT_SIZE 4 + 1 + 8 // 32 buttons -> 4bytes, 2 hats -> 1 byte, 4 axis * 2 bytes

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void tm_joystick_setup()
{
  // set defaults
  tm_joystick._buttonValues = NULL;
  tm_joystick._buttonValuesArraySize = 0;

  // Save Joystick Settings
  tm_joystick._buttonCount = JOYSTICK_DEFAULT_BUTTON_COUNT;
  tm_joystick._hatSwitchCount = JOYSTICK_DEFAULT_HATSWITCH_COUNT;

  // Setup Joystick State
  if (JOYSTICK_DEFAULT_BUTTON_COUNT > 0)
  {
    tm_joystick._buttonValuesArraySize = tm_joystick._buttonCount / 8;
    if ((tm_joystick._buttonCount % 8) > 0)
    {
      tm_joystick._buttonValuesArraySize++;
    }
    tm_joystick._buttonValues = malloc(sizeof(uint8_t[tm_joystick._buttonValuesArraySize]));
  }

  // Initialize Joystick State
  tm_joystick._xAxis = 0;
  tm_joystick._yAxis = 0;
  tm_joystick._zAxis = 0;
  tm_joystick._slider = 0;
  for (int index = 0; index < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; index++)
  {
    tm_joystick._hatSwitchValues[index] = JOYSTICK_HATSWITCH_RELEASE;
  }
  for (int index = 0; index < tm_joystick._buttonValuesArraySize; index++)
  {
    tm_joystick._buttonValues[index] = 0;
  }
}

void tm_joystick_setButton(uint8_t button, uint8_t value)
{
  if (value == 0)
  {
    tm_joystick_releaseButton(button);
  }
  else
  {
    tm_joystick_pressButton(button);
  }
}

void tm_joystick_pressButton(uint8_t button)
{
  if (button >= JOYSTICK_DEFAULT_BUTTON_COUNT)
    return;

  int index = button / 8;
  int bit = button % 8;

  bitSet(tm_joystick._buttonValues[index], bit);
}

void tm_joystick_releaseButton(uint8_t button)
{
  if (button >= JOYSTICK_DEFAULT_BUTTON_COUNT)
    return;

  int index = button / 8;
  int bit = button % 8;

  bitClear(tm_joystick._buttonValues[index], bit);
}

void tm_joystick_setXAxis(uint32_t value)
{
  tm_joystick._xAxis = value;
}

void tm_joystick_setYAxis(uint32_t value)
{
  tm_joystick._yAxis = value;
}

void tm_joystick_setZAxis(uint32_t value)
{
  tm_joystick._zAxis = value;
}

void tm_joystick_setSliderAxis(uint32_t value)
{
  tm_joystick._slider = value;
}

void tm_joystick_setHatSwitch(uint8_t hatSwitchIndex, uint16_t value)
{
  if (hatSwitchIndex >= JOYSTICK_DEFAULT_HATSWITCH_COUNT)
  {
    return;
  }

  tm_joystick._hatSwitchValues[hatSwitchIndex] = value;
}

uint32_t buildAndSet16BitValue(uint32_t value, uint32_t valueMinimum, uint32_t valueMaximum, uint32_t actualMinimum, uint32_t actualMaximum, uint8_t dataLocation[])
{
  uint32_t convertedValue;
  uint8_t highByte;
  uint8_t lowByte;
  uint32_t realMinimum = min(valueMinimum, valueMaximum);
  uint32_t realMaximum = max(valueMinimum, valueMaximum);

  if (value < realMinimum)
  {
    value = realMinimum;
  }
  if (value > realMaximum)
  {
    value = realMaximum;
  }

  if (valueMinimum > valueMaximum)
  {
    // Values go from a larger number to a smaller number (e.g. 1024 to 0)
    value = realMaximum - value + realMinimum;
  }

  convertedValue = map(value, realMinimum, realMaximum, actualMinimum, actualMaximum);

  highByte = (uint8_t)(convertedValue >> 8);
  lowByte = (uint8_t)(convertedValue & 0x00FF);

  dataLocation[0] = lowByte;
  dataLocation[1] = highByte;

  return 2;
}

uint32_t buildAndSetAxisValue(uint32_t axisValue, uint32_t axisMinimum, uint32_t axisMaximum, uint8_t dataLocation[])
{
  return buildAndSet16BitValue(axisValue, axisMinimum, axisMaximum, JOYSTICK_AXIS_MINIMUM, JOYSTICK_AXIS_MAXIMUM, dataLocation);
}

void tm_joystick_fill_report(tm_joystick_report *report)
{
  int index = 0;

  // Load Button State
  for (; index < tm_joystick._buttonValuesArraySize; index++)
  {
    report->buttons[index] = tm_joystick._buttonValues[index];
  }

  // Set Hat Switch Values
  if (JOYSTICK_DEFAULT_HATSWITCH_COUNT > 0)
  {

    // Calculate hat-switch values
    uint8_t convertedHatSwitch[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
    for (int hatSwitchIndex = 0; hatSwitchIndex < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; hatSwitchIndex++)
    {
      if (tm_joystick._hatSwitchValues[hatSwitchIndex] < 0)
      {
        convertedHatSwitch[hatSwitchIndex] = 8;
      }
      else
      {
        convertedHatSwitch[hatSwitchIndex] = (tm_joystick._hatSwitchValues[hatSwitchIndex] % 360) / 45;
      }
    }

    // Pack hat-switch states into a single byte
    report->hat[0] = (uint8_t)(convertedHatSwitch[1] << 4) | (0B00001111 & convertedHatSwitch[0]);

  } // Hat Switches

  // Set Axis Values
  buildAndSetAxisValue(tm_joystick._xAxis, 0, 65535, report->x);
  buildAndSetAxisValue(tm_joystick._yAxis, 0, 65535, report->y);
  buildAndSetAxisValue(tm_joystick._zAxis, 0, 4095, report->z);
  buildAndSetAxisValue(tm_joystick._slider, 0, 4095, report->s);
  
}
