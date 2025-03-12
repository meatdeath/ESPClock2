#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include <SPI.h>

#define NUMBER_OF_MATRIX        4

#define TIME_FORMAT_24H         0
#define TIME_FORMAT_12H         1

#define CLOCK_SHOW_TIME         6
#define TEMPERATURE_SHOW_TIME   3
#define PRESSURE_SHOW_TIME      3

//#define HIDE_HOUR_LEADING_ZERO

// ----------------------------------------------------------------------------

typedef enum matrix_orientation_en {
    MATRIX_ORIENTATION_0 = 0,
    MATRIX_ORIENTATION_CW90,
    MATRIX_ORIENTATION_CW180,
    MATRIX_ORIENTATION_CCW90
} matrix_orientation_t;



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
void display_Time(byte hours, byte minutes, byte seconds);

void display_StartingString(void);
void display_ClockString(void);
void display_VersionString(void);

//-----------------------------------------------------------------------------

extern matrix_orientation_t matrix_orientation;
extern uint8_t time_format;
extern String matrix_order;
extern bool display_show_leading_zero;

#endif // __DISPLAY_H__
