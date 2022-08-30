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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "tusb_config.h"

#include "usb_descriptors.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "display/ssd1306.h"
#include "mlx90333/mlx90333.h"

#define TEST_USB_SETUP 0

//--------------------------------------------------------------------+
// Display hardware setup
//--------------------------------------------------------------------+
#define DISPLAY_SDA_PIN 2
#define DISPLAY_SCL_PIN 3
#define DISPLAY_ADDRESS 0x3C
#define DISPLAY_I2C_INSTANCE (i2c1)
ssd1306_t disp;

//--------------------------------------------------------------------+
// MLX90333 sensor hardware setup
//--------------------------------------------------------------------+
#define MLX_90333_PIN_MISO 12
#define MLX_90333_PIN_MOSI 11
#define MLX_90333_PIN_SCK 10
#define MLX_90333_PIN_CS 13
#define MLX_90333_SPI_PORT (spi1)
mlx_90333_t hall_sensor;
mlx_90333_axis_data_t hall_sensor_data;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
  BLINK_NOT_MOUNTED = 1000,
  BLINK_MOUNTED = 250,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
tm_joystick_report lastReport = {
    .buttons = {0, 0, 0, 0},
    .hat = {0x11},
    .x = {0, 0},
    .y = {0, 0},
    .z = {0, 0},
    .s = {0, 0}};
tm_joystick_report newReport = {
    .buttons = {0, 0, 0, 0},
    .hat = {0x11},
    .x = {0, 0},
    .y = {0, 0},
    .z = {0, 0},
    .s = {0, 0}};

void led_blinking_task(void);
void hid_task(void);
void setup_display(void);
void setup_hall_sensor(void);

/*------------- MAIN -------------*/
int main(void)
{
  stdio_init_all();
  board_init();
  setup_hall_sensor();
  tm_joystick_setup();
  setup_display();
  tusb_init();
  ssd1306_update_display(&disp, 10000u, 10000u, 100u);

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// SSD1306 display setup
//--------------------------------------------------------------------+
void setup_display(void)
{
  ssd1306_init(&disp, 128, 64, DISPLAY_ADDRESS, DISPLAY_I2C_INSTANCE, DISPLAY_SDA_PIN, DISPLAY_SCL_PIN);
  bi_decl(bi_2pins_with_func(DISPLAY_SDA_PIN, DISPLAY_SCL_PIN, GPIO_FUNC_I2C))
}

//--------------------------------------------------------------------+
// MLX90333 sensor
//--------------------------------------------------------------------+
void setup_hall_sensor(void)
{
  mlx90333_setup(&hall_sensor, MLX_90333_SPI_PORT, MLX_90333_PIN_MISO, MLX_90333_PIN_MOSI, MLX_90333_PIN_SCK, MLX_90333_PIN_CS);
  // Make the SPI pins available to picotool
  bi_decl(bi_3pins_with_func(MLX_90333_PIN_MISO, MLX_90333_PIN_MOSI, MLX_90333_PIN_SCK, GPIO_FUNC_SPI))
      // Make the CS pin available to picotool
      bi_decl(bi_1pin_with_name(MLX_90333_PIN_CS, "SPI CS"))
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

void send_test_report(uint8_t testFunction)
{
  static uint32_t lastTestFunction = 0;
  static uint8_t currentButton = 31;
  static int16_t hat1Value = 0;
  static int16_t hat2Value = 0;
  static int32_t xValue = 0;
  static int32_t yValue = 0;
  static int32_t zValue = 0;
  static int32_t sValue = 0;

  tm_joystick_report report = {
      .buttons = {0, 0, 0, 0},
      .hat = {0x11},
      .x = {0, 0},
      .y = {0, 0},
      .z = {0, 0},
      .s = {0, 0}};

  switch (testFunction)
  {
  case 0: // buttons
    if (lastTestFunction == 6)
    {
      tm_joystick_setSliderAxis(0);
      lastTestFunction = 0;
    }
    tm_joystick_releaseButton(currentButton);
    currentButton++;
    if (currentButton > 31)
    {
      currentButton = 0;
    }
    tm_joystick_pressButton(currentButton);

    break;
  case 1: // hat 1
    if (lastTestFunction == 0)
    {
      tm_joystick_releaseButton(currentButton);
      lastTestFunction = 1;
    }
    tm_joystick_setHatSwitch(1, hat1Value * 45);
    hat1Value++;
    if (hat1Value > 7)
    {
      hat1Value = -1;
    }
    break;
  case 2: // hat 2
    if (lastTestFunction == 1)
    {
      tm_joystick_setHatSwitch(1, -1);
      lastTestFunction = 2;
    }
    tm_joystick_setHatSwitch(2, hat2Value * 45);
    hat2Value++;
    if (hat2Value > 7)
    {
      hat2Value = -1;
    }
    break;
  case 3: // x
    if (lastTestFunction == 2)
    {
      tm_joystick_setHatSwitch(2, -1);
      lastTestFunction = 3;
    }
    tm_joystick_setXAxis(xValue);
    xValue += 0x0300;
    if (xValue > 0xffff)
    {
      xValue = 0;
    }
    break;
  case 4: // y
    if (lastTestFunction == 3)
    {
      tm_joystick_setXAxis(0);
      lastTestFunction = 4;
    }
    tm_joystick_setYAxis(yValue);
    yValue += 0x0300;
    if (yValue > 0xffff)
    {
      yValue = 0;
    }
    break;
  case 5: // z
    if (lastTestFunction == 4)
    {
      tm_joystick_setYAxis(0);
      lastTestFunction = 5;
    }
    tm_joystick_setZAxis(zValue);
    zValue += 68;
    if (zValue > 4095)
    {
      zValue = 0;
    }
    break;
  case 6: // s
    if (lastTestFunction == 5)
    {
      tm_joystick_setZAxis(0);
      lastTestFunction = 6;
    }
    tm_joystick_setSliderAxis(sValue);
    sValue += 68;
    if (sValue > 4095)
    {
      sValue = 0;
    }
    break;
  default:
    break;
  }
  tm_joystick_fill_report(&report);
  tud_hid_report(0, &report, sizeof(report));
}

void send_hid_report(tm_joystick_report *report)
{
  tud_hid_report(0, report, sizeof(*report));
}

bool reportDataChanged(const tm_joystick_report *last_Report, const tm_joystick_report *new_Report)
{
  if (last_Report->buttons[0] != new_Report->buttons[0])
  {
    return true;
  }
  if (last_Report->buttons[1] != new_Report->buttons[1])
  {
    return true;
  }
  if (last_Report->buttons[2] != new_Report->buttons[2])
  {
    return true;
  }
  if (last_Report->buttons[3] != new_Report->buttons[3])
  {
    return true;
  }
  if (last_Report->hat[0] != new_Report->hat[0])
  {
    return true;
  }
  if (last_Report->s[0] != new_Report->s[0])
  {
    return true;
  }
  if (last_Report->s[1] != new_Report->s[1])
  {
    return true;
  }
  if (last_Report->x[0] != new_Report->x[0])
  {
    return true;
  }
  if (last_Report->x[1] != new_Report->x[1])
  {
    return true;
  }
  if (last_Report->y[0] != new_Report->y[0])
  {
    return true;
  }
  if (last_Report->y[1] != new_Report->y[1])
  {
    return true;
  }
  if (last_Report->z[0] != new_Report->z[0])
  {
    return true;
  }
  if (last_Report->z[1] != new_Report->z[1])
  {
    return true;
  }

  return false;
}

void copy_report_values(const tm_joystick_report *source, tm_joystick_report *destination)
{
  destination->buttons[0] = source->buttons[0];
  destination->buttons[1] = source->buttons[1];
  destination->buttons[2] = source->buttons[2];
  destination->buttons[3] = source->buttons[3];
  destination->hat[0] = source->hat[0];
  destination->s[0] = source->s[0];
  destination->s[1] = source->s[1];
  destination->x[0] = source->x[0];
  destination->x[1] = source->x[1];
  destination->y[0] = source->y[0];
  destination->y[1] = source->y[1];
  destination->z[0] = source->z[0];
  destination->z[1] = source->z[1];
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  static const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if (board_millis() - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  if (!tud_suspended())
  {
    // skip if hid is not ready yet
    if (!tud_hid_ready())
    {
      blink_interval_ms = 0;
      return;
    }
    blink_interval_ms = BLINK_MOUNTED;

#if TEST_USB_SETUP > 0
    static uint8_t testFunction = 0;
    uint32_t const btn = board_button_read();
    if (btn)
    {
      testFunction++;
      if (testFunction > 6)
      {
        testFunction = 0;
      }
    }
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_test_report(testFunction);
#else
    // read hall sensor data
    mlx90333_get_axis_data(&hall_sensor, &hall_sensor_data);
    if(hall_sensor_data.valid)
    {
      tm_joystick_setXAxis(hall_sensor_data.x);
      tm_joystick_setYAxis(hall_sensor_data.y);
    }

    tm_joystick_fill_report(&newReport);

    //if(reportDataChanged(&lastReport, &newReport))
    {
      tud_hid_report(0, &newReport, sizeof(newReport));
      copy_report_values(&newReport, &lastReport);
    }

    ssd1306_update_display(&disp, hall_sensor_data.x, hall_sensor_data.y, newReport.z);
    //ssd1306_debug_values(&disp, hall_sensor_data.x, hall_sensor_data.y, hall_sensor_data.valid);
    if(!hall_sensor_data.valid){
      sleep_ms(500);
    }
#endif
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
  (void)instance;
  (void)len;
  (void)report;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms)
    return;

  // Blink every interval ms
  if (board_millis() - start_ms < blink_interval_ms)
    return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = !led_state; // toggle
}
