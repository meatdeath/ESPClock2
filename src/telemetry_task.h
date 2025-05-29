#ifndef __BMP_H__
#define __BMP_H__

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

typedef struct telemetry_st 
{
    bool valid;
    float temperature;
    float pressure;
    float humidity;
} 
telemetry_t;

extern Task TelemetryTask;
extern telemetry_t telemetry;

bool TelemetryInit();

#endif // __BMP_H__
