#ifndef _tmext_josystick_h
#define _tmext_josystick_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

// values addresses
#define BTN0_7 0
#define BTN8_15 1
#define BTN16_23 2
#define BTN24_31 3
#define BTN32_39 4
#define BTN40_47 5
#define BTN48_55 6
#define BTN56_63 7
#define BTN64_71 8
#define BTN72_79 9
#define BTN80_87 10
#define BTN88_95 11
#define BTN96_103 12
#define BTN104_111 13
#define BTN112_119 14
#define BTN120_127 15

#define X_AXIS_LSB 16
#define X_AXIS_MSB 17
#define Y_AXIS_LSB 18
#define Y_AXIS_MSB 19

#define HAT0_1 20 // Hats are 4 bit direction (0-9), 2 hats per byte
#define HAT2_3 21

#define Z_AXIS_LSB 22
#define Z_AXIS_MSB 23
#define Rx_AXIS_LSB 24
#define Rx_AXIS_MSB 25
#define Ry_AXIS_LSB 26
#define Ry_AXIS_MSB 27
#define Rz_AXIS_LSB 28
#define Rz_AXIS_MSB 29

#define THROTTLE_AXIS_LSB 30
#define THROTTLE_AXIS_MSB 31
// #define S0_AXIS_LSB 30
// #define S0_AXIS_MSB 31
#define S0_AXIS_LSB 32
#define S0_AXIS_MSB 33

#define HAT_DIR_N 0
#define HAT_DIR_NE 1
#define HAT_DIR_E 2
#define HAT_DIR_SE 3
#define HAT_DIR_S 4
#define HAT_DIR_SW 5
#define HAT_DIR_W 6
#define HAT_DIR_NW 7
#define HAT_DIR_C 8

    bool send_inputs(uint8_t *values);

    bool test_send(uint16_t b0, uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7,
                   uint16_t x, uint16_t y, uint16_t hats, uint16_t z, uint16_t Rx, uint16_t Ry, uint16_t Rz, uint16_t s0, uint16_t s1);

    bool randomizeInputs();

    void SetButton(int idx, bool val);
    void SetX(uint16_t val);
    void SetY(uint16_t val);
    void SetZ(uint16_t val);
    void SetRx(uint16_t val);
    void SetRy(uint16_t val);
    void SetRz(uint16_t val);
    void SetS0(uint16_t val);
    void SetThrottle(uint16_t val);
    void SetS1(uint16_t val);

    // 4 Hats available 0-3, direction is clockwise 0=N 1=NE 2=E 3=SE 4=S 5=SW 6=W 7=NW 8=CENTER
    void SetHat(uint8_t hatIdx, uint8_t dir);

    bool send_update();
    /*
     * To define the report descriptor. Warning: this method has to store the length of the report descriptor in reportLength.
     *
     * @returns pointer to the report descriptor
     */
    const uint8_t *report_desc();
    /*
     * Get configuration descriptor
     *
     * @returns pointer to the configuration descriptor
     */
    const uint8_t *configuration_desc(uint8_t index);

    uint8_t inputArray[35];

    uint8_t _configuration_descriptor[41];

#ifdef __cplusplus
}
#endif

#endif /* _tmext_josystick_h */