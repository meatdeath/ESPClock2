#include "main.h"

// ----------------------------------------------------------------------------

#ifdef ESP32
#define SPI_DATA_PIN    23
#define SPI_CS_PIN      22
#define SPI_CLK_PIN     18
#else // ESP8266
#define SPI_DATA_PIN    D8
#define SPI_CS_PIN      D7
#define SPI_CLK_PIN     D5
#endif

#define DISPLAY_BUF_SIZE    (8*NUMBER_OF_MATRIX)

uint8_t display_buf[DISPLAY_BUF_SIZE] = {0};
MATRIX7219 mx(SPI_DATA_PIN, SPI_CS_PIN, SPI_CLK_PIN, NUMBER_OF_MATRIX);

// setup variables
matrix_orientation_t matrix_orientation = MATRIX_ORIENTATION_CW90;
bool matrix_straight_order = false;
bool display_hide_leading_zero = false;

bool time_format = TIME_FORMAT_24H;

// ----------------------------------------------------------------------------

#pragma pack(push,1)

typedef struct _digits_st 
{
    byte size;
    byte array[5];
} 
digits_t;

typedef struct _screens_st 
{
    byte size;
    byte array[32];
} 
screens_t;

typedef union _display_item_un 
{
    digits_t digit;
    screens_t screen;
} 
display_item_t;

#pragma pack(pop)

// ----------------------------------------------------------------------------

const digits_t digits [] PROGMEM = 
{
    { 5, { 0b01111110, 
           0b10000001,
           0b10000001, 
           0b10000001, 
           0b01111110 } },   //0

    { 5, { 0b00000000, 
           0b00000100, 
           0b00000010, 
           0b11111111, 
           0b00000000 } },   //1

    { 5, { 0b10000010, 
           0b11000001, 
           0b10100001, 
           0b10010001, 
           0b10001110 } },   //2

    { 5, { 0b01000010, 
           0b10000001, 
           0b10001001, 
           0b10001001, 
           0b01110110 } },   //3

    { 5, { 0b00110000, 
           0b00101000, 
           0b00100100, 
           0b00100010, 
           0b11111111 } },   //4

    { 5, { 0b01001111, 
           0b10001001, 
           0b10001001, 
           0b10001001, 
           0b01110001 } },   //5

    { 5, { 0b01111110, 
           0b10001001, 
           0b10001001, 
           0b10001001, 
           0b01110010 } },   //6

    { 5, { 0b00000001, 
           0b11100001, 
           0b00010001, 
           0b00001001, 
           0b00000111 } },   //7

    { 5, { 0b01110110, 
           0b10001001, 
           0b10001001, 
           0b10001001, 
           0b01110110 } },   //8

    { 5, { 0b01001110, 
           0b10010001, 
           0b10010001, 
           0b10010001, 
           0b01111110 } },   //9

    { 1, { 0b00100100 } },   //:

    { 2, { 0b00000011, 
           0b00000011 } },   //.

    { 3, { 0x10, 0x10, 0x10 } },   // -
    { 4, { 0x06, 0x09, 0x09, 0x06 } },   // Degree symbol
    { 5, { 0x7E, 0x81, 0x81, 0x81, 0x42 } },   // C (Celcius)
    { 5, { 0xF8, 0x30, 0x40, 0x30, 0xF8 } },   // m
};

// ----------------------------------------------------------------------------

const screens_t screens [] PROGMEM = 
{
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
            0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x10, 0x10, 0x10, 0xff, 0x00, 0xf8, 0x24, 0x24, 0xfc, 0x00,
            0x78, 0x84, 0x84, 0x48, 0x00, 0xfc, 0x90, 0x90, 0x60, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00
        }
    },
};

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
    if (!matrix_straight_order)
    {
        uint8_t pos = row%8;
        uint8_t matrix = (NUMBER_OF_MATRIX-1) - (row/8);
        row = matrix*8 + pos;
    }
    display_buf[row] = data;
}

// ----------------------------------------------------------------------------

void display_Commit()
{
    if (matrix_orientation == MATRIX_ORIENTATION_0)
    {
        for (int i = 0; i < DISPLAY_BUF_SIZE; i++)
        {
            mx.setRow(i%8+1, display_buf[i], i/8+1);
        }
    }
    else if (matrix_orientation == MATRIX_ORIENTATION_CW180)
    {
        for (int m = 0; m < NUMBER_OF_MATRIX; m++)
        {
            for (int i = 0; i < 8; i++)
            {
                mx.setRow(7-i+1, BitReverse(display_buf[m*8+i]), m+1);
            }
        }
    }
    else if (matrix_orientation == MATRIX_ORIENTATION_CW90)
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
                mx.setRow(i+1, data, m+1);
            }
        }
    }
    else // if (matrix_orientation == MATRIX_ORIENTATION_CCW90)
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
                mx.setRow(i+1, data, m+1);
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
    uint8_t size;
    uint8_t i, offset = 0;
    
    uint8_t symbols[] = 
    {
        (uint8_t)((pressure/100)%10),
        (uint8_t)((pressure/10)%10),
        (uint8_t)(pressure%10),
        DISPLAY_SYMBOL_M,
        DISPLAY_SYMBOL_M
    };
    
    display_Clear();

    for(int j = 0; j < 4; j++) 
    {
        size = pgm_read_byte(&(digits[symbols[j]].size));
        for( i = 0; i < size; i++ ) 
        {
            display_Row(offset+i, pgm_read_byte(&(digits[symbols[j]].array[i])));
        }
        offset += size + 2;        
    }
    
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_Temperature(int temperature) 
{
    uint8_t i, offset = 0;
    bool negative = false;

    display_Clear();

    uint8_t size;
    if( temperature < 0 ) 
    {
        negative = true;
        temperature = -temperature;
    }

    uint8_t t1 = temperature / 10;
    uint8_t t2 = temperature % 10;
    if( t1 == 0 ) 
    {
        size = pgm_read_byte(&(digits[t1].size));
        for( i = 0; i < size; i++ ) 
        {
            display_Row(offset+i, 0x00);
        }
        offset += size + 2;
    }

    size = pgm_read_byte(&(digits[DISPLAY_SYMBOL_DASH].size));
    if( negative )
    {
        for( i = 0; i < size; i++ ) 
        {
            display_Row(offset+i, pgm_read_byte(&(digits[DISPLAY_SYMBOL_DASH].array[i])));
        }
    } 
    else 
    {
        for( i = 0; i < size; i++ ) 
        {
            display_Row(offset+i, 0x00);
        }
    }
    offset += size + 2;

    if( t1 ) 
    {
        size = pgm_read_byte(&(digits[t1].size));
        for (i = 0; i < size; i++) 
        {
            display_Row(offset+i, pgm_read_byte(&(digits[t1].array[i])));
        }
        offset += size + 2;
    }

    size = pgm_read_byte(&(digits[t2].size));
    for (i = 0; i < size; i++) 
    {
        display_Row(offset+i, pgm_read_byte(&(digits[t2].array[i])));
    }
    offset += size + 2;
    size = pgm_read_byte(&(digits[DISPLAY_SYMBOL_C].size));
    for( i = 0; i < size; i++ ) 
    {
        display_Row(offset+i, pgm_read_byte(&(digits[DISPLAY_SYMBOL_C].array[i])));
    }
    
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_Time(byte hours, byte minutes, byte seconds) 
{
    byte i;
    if (time_format == TIME_FORMAT_12H) hours %= 12;
    else hours %= 24;
    byte h1 = hours/10;
    byte h2 = hours%10;
    byte m1 = minutes/10;
    byte m2 = minutes%10;
    display_Clear();

    if (h1 == 0 && display_hide_leading_zero)
    {
        for (i = 0; i < pgm_read_byte(&(digits[h1].size)); i++) 
        {
            display_Row(i, 0);
        }
    }
    else
    {
        for (i = 0; i < pgm_read_byte(&(digits[h1].size)); i++) 
        {
            display_Row(i, pgm_read_byte(&(digits[h1].array[i])));
        }
    }
    
    //display_Row(i,0); // space
    // hours low
    for (i = 0; i < pgm_read_byte(&(digits[h2].size)); i++) 
    {
        display_Row(7+i, pgm_read_byte(&(digits[h2].array[i])));
    }
    //display_Row(7+i,0); // space
    // colon
    for (i = 0; i < digits[DISPLAY_SYMBOL_COLON].size; i++) 
    {
        display_Row(15+i, (seconds&1)?pgm_read_byte(&(digits[DISPLAY_SYMBOL_COLON].array[i])):0);
    }
    //display_Row(15+i,0); // space
    // minutes high
    for (i = 0; i < pgm_read_byte(&(digits[m1].size)); i++) 
    {
        display_Row(19+i, pgm_read_byte(&(digits[m1].array[i])));
    }
    //display_Row(19+i,0); // space
    // minutes low
    for (i = 0; i < pgm_read_byte(&(digits[m2].size)); i++) 
    {
        display_Row(26+i, pgm_read_byte(&(digits[m2].array[i])));
    }
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_StartingString(void) 
{
    display_Clear();
    for (byte i = 0; i < pgm_read_byte(&(screens[DISPLAY_STARTING].size)); i++) 
    {
        display_Row(i, pgm_read_byte(&(screens[DISPLAY_STARTING].array[i])));
    }
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_ClockString(void) 
{
    display_Clear();
    for (byte i = 0; i < pgm_read_byte(&(screens[DISPLAY_CLOCK_STR].size)); i++) 
    {
        display_Row(i, pgm_read_byte(&(screens[DISPLAY_CLOCK_STR].array[i])));
    }
    display_Commit();
}

// ----------------------------------------------------------------------------

void display_VersionString(void) 
{
    display_Clear();
    const byte v_dot[] = {0xfc,0x94,0x94,0x94,0x68,0x00,0x80,0x00};
    byte j = 7;
    for (byte i = 0; i < sizeof(v_dot); i++, j++) 
    {
        display_Row(j, v_dot[i]);
    }
    for (byte i = 0; i < pgm_read_byte(&(digits[VERSION_MAJOR].size)); i++, j++) 
    {
        display_Row(j, pgm_read_byte(&(digits[VERSION_MAJOR].array[i])));
    }
    display_Row(j++, 0x00);
    display_Row(j++, 0x80); // dot
    display_Row(j++, 0x00);
    for (byte i = 0; i < pgm_read_byte(&(digits[VERSION_MINOR].size)); i++, j++) 
    {
        display_Row(j, pgm_read_byte(&(digits[VERSION_MINOR].array[i])));
    }
    while( j < 32 ) 
    {
        display_Row(j++, 0x00);
    }
    display_Commit();
}

// ----------------------------------------------------------------------------
