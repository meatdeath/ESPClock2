#ifndef __BMP_H__
#define __BMP_H__

typedef struct telemetry_st 
{
    bool valid;
    float temperature;
    float pressure;
} 
telemetry_t;

extern Task bmp280Task;
extern telemetry_t telemetry;

bool bmp280Init();

#endif // __BMP_H__
