#ifndef __TELEMETRY_LOG_H__
#define __TELEMETRY_LOG_H__

#pragma pack(1)
typedef struct SensorRecord {
    uint32_t time;
    float temperature;
    float pressure;
    float humidity;
} sensor_record_t;
#pragma pack()
  
#define SHORT_TERM_COUNT   48
#define MID_TERM_COUNT     28
#define LONG_TERM_COUNT    30

#define SHORT_TERM_FILE    "/short.bin"
#define MID_TERM_FILE      "/mid.bin"
#define LONG_TERM_FILE     "/long.bin"

//-----------------------------------------------------------------------------

void initTelemetryLog();
void logTelemetry(uint32_t epochTime, float temp, float pressure, float humidity);
void printBuffer(const char* filename, uint16_t count);
void writeRecordToBuffer(const char* filename, SensorRecord record, uint16_t index, uint16_t maxCount);

//-----------------------------------------------------------------------------

#endif // __TELEMETRY_LOG_H__
