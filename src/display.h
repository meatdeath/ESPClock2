#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include <SPI.h>

#define NUMBER_OF_MATRIX        4

#define DISPLAY_FORMAT_24H      0
#define DISPLAY_FORMAT_12H      1

#define CLOCK_SHOW_TIME         6
#define TEMPERATURE_SHOW_TIME   3
#define PRESSURE_SHOW_TIME      3

#ifdef ESP8266
#define CS_PIN                  15
#else
#define CS_PIN                  0
#endif
//#define HIDE_HOUR_LEADING_ZERO

#pragma pack(push,1)

typedef struct _digits_st {
    byte size;
    byte array[5];
} digits_t;

typedef struct _screens_st {
    byte size;
    byte array[32];
} screens_t;

typedef union _display_item_un {
    digits_t digit;
    screens_t screen;
} display_item_t;

#pragma pack(pop)

// ----------------------------------------------------------------------------

typedef enum display_orientation_en {
    DISPLAY_ORIENTATION_0 = 0,
    DISPLAY_ORIENTATION_CW90,
    DISPLAY_ORIENTATION_CW180,
    DISPLAY_ORIENTATION_CCW90
} display_orientation_t;


const digits_t digits [] PROGMEM = {
    { 5, { 0b01111110, 0b10000001, 0b10000001, 0b10000001, 0b01111110 } },   //0
    { 5, { 0b00000000, 0b00000100, 0b00000010, 0b11111111, 0b00000000 } },   //1
    { 5, { 0b10000010, 0b11000001, 0b10100001, 0b10010001, 0b10001110 } },   //2
    { 5, { 0b01000010, 0b10000001, 0b10001001, 0b10001001, 0b01110110 } },   //3
    { 5, { 0b00110000, 0b00101000, 0b00100100, 0b00100010, 0b11111111 } },   //4
    { 5, { 0b01001111, 0b10001001, 0b10001001, 0b10001001, 0b01110001 } },   //5
    { 5, { 0b01111110, 0b10001001, 0b10001001, 0b10001001, 0b01110010 } },   //6
    { 5, { 0b00000001, 0b11100001, 0b00010001, 0b00001001, 0b00000111 } },   //7
    { 5, { 0b01110110, 0b10001001, 0b10001001, 0b10001001, 0b01110110 } },   //8
    { 5, { 0b01001110, 0b10010001, 0b10010001, 0b10010001, 0b01111110 } },   //9
    { 1, { 0b00100100 } },   //:
    { 2, { 0b00000011, 0b00000011 } },   //.
    { 3, { 0x10, 0x10, 0x10 } },   // -
    { 4, { 0x06, 0x09, 0x09, 0x06 } },   // Degree symbol
    { 5, { 0x7E, 0x81, 0x81, 0x81, 0x42 } },   // C (Celcius)
    { 5, { 0xF8, 0x30, 0x40, 0x30, 0xF8 } },   // m
};

enum __display_symbols {
    DISPLAY_SYMBOL_COLON = 10,
    DISPLAY_SYMBOL_DOT,
    DISPLAY_SYMBOL_DASH,
    DISPLAY_SYMBOL_DEGREE,
    DISPLAY_SYMBOL_C,
    DISPLAY_SYMBOL_M,
};

const screens_t screens [] PROGMEM = {
    {   // Zapusk...
        32,
        {
            0x22, 0x49, 0x49, 0x36, 0x00, 0x78, 0x24, 0x24, 0x78, 0x00, 0x7c, 0x04, 0x04, 0x7c, 0x00, 0x0c,
            0x50, 0x50, 0x3c, 0x00, 0x38, 0x44, 0x44, 0x28, 0x00, 0x7c, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00
        }
    },
    {   // Starting...
        32,
        {
            0x7E, 0x81, 0x81, 0x81, 0x62, 0x00, 0x04, 0xfC, 0x04, 0x00, 0xF8, 0x24, 0x24, 0xFC, 0x00, 0xFC, 
            0x24, 0x24, 0x18, 0x00, 0x04, 0xfC, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80 
        }
    },
    {   // Clock
        32, 
        {
            0x00,0x00,0x00,0x00,0x00,0x0f,0x10,0x10,0x10,0xff,0x00,0xf8,0x24,0x24,0xfc,0x00,
            0x78,0x84,0x84,0x48,0x00,0xfc,0x90,0x90,0x60,0x00,0xfc,0x00,0x00,0x00,0x00,0x00
        }
    },
};

enum __display_screens {
    DISPLAY_SYMBOL_ZAPUSK,
    DISPLAY_STARTING,
    DISPLAY_CLOCK_STR
};

//-----------------------------------------------------------------------------

void display_Commit(void);
void display_Init(void);
void display_Clear(void);
void display_SetIntensity(byte intensity);
void display_SetBrightness(uint8_t percentage);

void display_Pressure(uint16_t pressure);
void display_Temperature(int temperature);
void display_Time(byte hours, byte minutes, byte seconds, byte format);

void display_StartingString(void);
void display_ClockString(void);
void display_VersionString(void);

//-----------------------------------------------------------------------------

extern display_orientation_t display_orientation;

#endif // __DISPLAY_H__
