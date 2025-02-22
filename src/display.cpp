#include "main.h"


// ----------------------------------------------------------------------------

#ifdef ESP32
uint8_t dataPin   = 21;
uint8_t selectPin = 22;
uint8_t clockPin  = 23;
uint8_t count     = 1;
#else

#endif

MATRIX7219 mx(dataPin, selectPin, clockPin, NUMBER_OF_MATRIX);

display_orientation_t display_orientation = DISPLAY_ORIENTATION_0;

#define DISPLAY_BUF_SIZE    (8*NUMBER_OF_MATRIX)

uint8_t display_buf[DISPLAY_BUF_SIZE] = {0};

// ----------------------------------------------------------------------------

// Reverse the order of bits in a byte. 
// I.e. MSB is swapped with LSB, etc. 
uint8_t BitReverse(uint8_t x) 
{ 
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa); 
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc); 
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0); 
    return x;    
}

// ----------------------------------------------------------------------------

void display_Row(uint8_t row, uint8_t data)
{
    display_buf[row] = data;
}

// ----------------------------------------------------------------------------

void display_Commit()
{
    if (display_orientation == DISPLAY_ORIENTATION_0)
    {
        for (int i = 0; i < DISPLAY_BUF_SIZE; i++)
        {
            mx.setRow(i%8, display_buf[i], i/8);
        }
    }
    else if (display_orientation == DISPLAY_ORIENTATION_CW180)
    {
        for (int m = 0; m < NUMBER_OF_MATRIX; m++)
        {
            for (int i = 0; i < 8; i++)
            {
                mx.setRow(7-i, BitReverse(display_buf[m*8+i]), m);
            }
        }
    }
    else if (display_orientation == DISPLAY_ORIENTATION_CW90)
    {
        for (int m = 0; m < NUMBER_OF_MATRIX; m++)
        {
            for (int i = 0; i < 8; i++)
            {
                uint8_t mask = 0x80>>i;
                uint8_t data = 0;
                for (int j = 0; j < 8; j++)
                {
                    if (display_buf[m*8+j]&mask)
                        data |= 1<<j;
                }
                mx.setRow(i, data, m);
            }
        }
    }
    else // if (display_orientation == DISPLAY_ORIENTATION_CCW90)
    {
        for (int m = 0; m < NUMBER_OF_MATRIX; m++)
        {
            for (int i = 0; i < 8; i++)
            {
                uint8_t mask = 0x01<<i;
                uint8_t data = 0;
                for (int j = 0; j < 8; j++)
                {
                    if (display_buf[m*8+j]&mask)
                        data |= 0x80>>j;
                }
                mx.setRow(i, data, m);
            }
        }
    }
}

// ----------------------------------------------------------------------------

void display_Init(void) 
{
    mx.begin();
    mx.clear();
    mx.setBrightness(0);
}

// ----------------------------------------------------------------------------

void display_Clear()
{
    memset(display_buf, 0, DISPLAY_BUF_SIZE);
    mx.clear();
}

// ----------------------------------------------------------------------------

void display_SetIntensity(byte intensity)
{
    mx.setBrightness(intensity);
}

// ----------------------------------------------------------------------------

void display_SetBrightness(uint8_t percentage)
{
    if (percentage <= 6)       mx.setBrightness(0);
    else if (percentage <= 12) mx.setBrightness(1);
    else if (percentage <= 18) mx.setBrightness(2);
    else if (percentage <= 25) mx.setBrightness(3);
    else if (percentage <= 31) mx.setBrightness(4);
    else if (percentage <= 37) mx.setBrightness(5);
    else if (percentage <= 43) mx.setBrightness(6);
    else if (percentage <= 50) mx.setBrightness(7);
    else if (percentage <= 56) mx.setBrightness(8);
    else if (percentage <= 62) mx.setBrightness(9);
    else if (percentage <= 68) mx.setBrightness(10);
    else if (percentage <= 75) mx.setBrightness(11);
    else if (percentage <= 81) mx.setBrightness(12);
    else if (percentage <= 87) mx.setBrightness(13);
    else if (percentage <= 93) mx.setBrightness(14);
    else                       mx.setBrightness(15);
}

// ----------------------------------------------------------------------------

void display_Pressure(uint16_t pressure) 
{
    uint8_t i, offset = 0;
    uint8_t symbols[] = {
        (uint8_t)((pressure/100)%10),
        (uint8_t)((pressure/10)%10),
        (uint8_t)(pressure%10),
        DISPLAY_SYMBOL_M,
        DISPLAY_SYMBOL_M
    };
    uint8_t size;
    
    display_Clear();

    for(int j = 0; j < 4; j++) {
        size = pgm_read_byte(&(digits[symbols[j]].size));
        for( i = 0; i < size; i++ ) {
            display_Row(offset+i, pgm_read_byte(&(digits[symbols[j]].array[i])) );
        }
        offset += size + 2;        
    }
    
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_Temperature(int temperature) {
    uint8_t i, offset = 0;
    bool negative = false;

    display_Clear();

    uint8_t size;
    if( temperature < 0 ) {
        negative = true;
        temperature = -temperature;
    }

    uint8_t t1 = temperature / 10;
    uint8_t t2 = temperature % 10;
    if( t1 == 0 ) {
        size = pgm_read_byte(&(digits[t1].size));
        for( i = 0; i < size; i++ ) {
            display_Row(offset+i, 0x00 );
        }
        offset += size + 2;
    }

    size = pgm_read_byte(&(digits[DISPLAY_SYMBOL_DASH].size));
    if( negative ) {
        for( i = 0; i < size; i++ ) {
            display_Row(offset+i, pgm_read_byte(&(digits[DISPLAY_SYMBOL_DASH].array[i])) );
        }
    } else {
        for( i = 0; i < size; i++ ) {
            display_Row(offset+i, 0x00);
        }
    }
    offset += size + 2;


    if( t1 ) {
        size = pgm_read_byte(&(digits[t1].size));
        for( i = 0; i < size; i++ ) {
            display_Row(offset+i, pgm_read_byte(&(digits[t1].array[i])) );
        }
        offset += size + 2;
    }

    size = pgm_read_byte(&(digits[t2].size));
    for( i = 0; i < size; i++ ) {
        display_Row(offset+i, pgm_read_byte(&(digits[t2].array[i])) );
    }
    offset += size + 2;
    size = pgm_read_byte(&(digits[DISPLAY_SYMBOL_C].size));
    for( i = 0; i < size; i++ ) {
        display_Row(offset+i, pgm_read_byte(&(digits[DISPLAY_SYMBOL_C].array[i])) );
    }
    
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_Time(byte hours, byte minutes, byte seconds, byte format) 
{
    byte i;

    if( format == DISPLAY_FORMAT_12H ) hours %= 12;
    else hours %= 24;

    byte h1 = hours/10;
    byte h2 = hours%10;
    byte m1 = minutes/10;
    byte m2 = minutes%10;
    
    display_Clear();
#ifndef HIDE_HOUR_LEADING_ZERO
    if( h1 != 0 ) {
#endif
        for( i = 0; i < pgm_read_byte(&(digits[h1].size)); i++ ) {
            display_Row(i, pgm_read_byte(&(digits[h1].array[i])) );
        }
#ifndef HIDE_HOUR_LEADING_ZERO
    }else {
        for( i = 0; i < pgm_read_byte(&(digits[h1].size)); i++ ) {
            display_Row(i, 0 );
        }
    }
#endif
    //ledMatrix.setColumn(i,0); // space
    //hours low
    for( i = 0; i < pgm_read_byte(&(digits[h2].size)); i++ ) {
        display_Row(7+i, pgm_read_byte(&(digits[h2].array[i])) );
    }
    //ledMatrix.setColumn(7+i,0); // space
    //colon
    for( i = 0; i < digits[DISPLAY_SYMBOL_COLON].size; i++ ) {
        display_Row(15+i, (seconds&1)?pgm_read_byte(&(digits[DISPLAY_SYMBOL_COLON].array[i])):0 );
    }
    //ledMatrix.setColumn(15+i,0); // space
    // minutes high
    for( i = 0; i < pgm_read_byte(&(digits[m1].size)); i++ ) {
        display_Row(19+i, pgm_read_byte(&(digits[m1].array[i])) );
    }
    //ledMatrix.setColumn(19+i,0); // space
    // minutes low
    for( i = 0; i < pgm_read_byte(&(digits[m2].size)); i++ ) {
        display_Row(26+i, pgm_read_byte(&(digits[m2].array[i])) );
    }
    
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_StartingString(void) {
    for( byte i = 0; i < pgm_read_byte(&(screens[DISPLAY_STARTING].size)); i++ ) {
        display_Row(i, pgm_read_byte(&(screens[DISPLAY_STARTING].array[i])) );
    }

    display_Commit();
}

// ----------------------------------------------------------------------------

void display_ClockString(void) {
    for( byte i = 0; i < pgm_read_byte(&(screens[DISPLAY_CLOCK_STR].size)); i++ ) {
        display_Row(i, pgm_read_byte(&(screens[DISPLAY_CLOCK_STR].array[i])) );
    }
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_VersionString(void) {
    const byte v_dot[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x94,0x94,0x94,0x68,0x00,0x80,0x00};
    byte j = 0;
    for( byte i = 0; i < sizeof(v_dot); i++, j++ ) {
        display_Row(j, v_dot[i]);
    }
    for( byte i = 0; i < pgm_read_byte(&(digits[VERSION_MAJOR].size)); i++, j++ ) {
        display_Row(j, pgm_read_byte(&(digits[VERSION_MAJOR].array[i])) );
    }
    display_Row(j++, 0x00);
    display_Row(j++, 0x80);
    display_Row(j++, 0x00);
    
    for( byte i = 0; i < pgm_read_byte(&(digits[VERSION_MINOR].size)); i++, j++ ) {
        display_Row(j, pgm_read_byte(&(digits[VERSION_MINOR].array[i])) );
    }

    while( j < 32 ) {
        display_Row(j++, 0x00);
    }
    
    display_Commit();
}

// ----------------------------------------------------------------------------
