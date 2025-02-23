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

// ----------------------------------------------------------------------------

typedef enum display_orientation_en {
    DISPLAY_ORIENTATION_0 = 0,
    DISPLAY_ORIENTATION_CW90,
    DISPLAY_ORIENTATION_CW180,
    DISPLAY_ORIENTATION_CCW90
} display_orientation_t;



enum __display_symbols {
    DISPLAY_SYMBOL_COLON = 10,
    DISPLAY_SYMBOL_DOT,
    DISPLAY_SYMBOL_DASH,
    DISPLAY_SYMBOL_DEGREE,
    DISPLAY_SYMBOL_C,
    DISPLAY_SYMBOL_M,
};

enum __display_screens {
    DISPLAY_SYMBOL_ZAPUSK,
    DISPLAY_STARTING,
    DISPLAY_CLOCK_STR
};

//-----------------------------------------------------------------------------

void display_Row(uint8_t row, uint8_t data);
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
