#include "main.h"

//-----------------------------------------------------------------------------

uint16_t shortIndex = 0;
uint16_t midIndex = 0;
uint16_t longIndex = 0;

unsigned long lastMidSave = 0;
unsigned long lastLongSave = 0;

void initBuffer(const char* filename, uint16_t count);

//-----------------------------------------------------------------------------

void initTelemetryLog() {
    initBuffer(SHORT_TERM_FILE, SHORT_TERM_COUNT);
    initBuffer(MID_TERM_FILE, MID_TERM_COUNT);
    initBuffer(LONG_TERM_FILE, LONG_TERM_COUNT);
}

//-----------------------------------------------------------------------------

void initBuffer(const char* filename, uint16_t count) {
    if (!LittleFS.exists(filename)) {
        Serial.println("File "+(String)filename+" doesn't exist, creating...");
        File file = LittleFS.open(filename, FILE_WRITE);
        for (int i = 0; i < count; i++) {
            SensorRecord empty = {0, 0, 0, 0};
            file.write((uint8_t*)&empty, sizeof(SensorRecord));
        }
        file.close();
    }
}

//-----------------------------------------------------------------------------

void writeRecordToBuffer(const char* filename, SensorRecord record, uint16_t index, uint16_t maxCount) {
    SensorRecord buffer[maxCount];
    File file = LittleFS.open(filename, FILE_READ);
    if (file) {
        file.read((uint8_t*)buffer, sizeof(buffer));
        file.close();
    }
    file = LittleFS.open(filename, FILE_WRITE);
    if (file) {
        file.write((uint8_t*)&buffer[1], sizeof(buffer)-sizeof(SensorRecord));
        file.write((uint8_t*)&record, sizeof(SensorRecord));
        file.close();
    }
}

//-----------------------------------------------------------------------------

void logTelemetry(uint32_t epochTime, float temp, float pressure, float humidity) {
    struct tm *timeInfo;
    timeInfo = localtime((time_t*)&epochTime);

   //  if (timeInfo->tm_min == 0 && timeInfo->tm_sec == 0) 
    {
        Serial.printf("Save telemetry data: %lu - %.2f/%.2f/%.0f\n",epochTime,temp,humidity,pressure);
        Serial.printf("Indexes: %d/%d/%d\n",shortIndex,midIndex,longIndex);
        SensorRecord record = { epochTime, temp, pressure, humidity };

        // Запись в краткосрочный буфер (каждый час)
        writeRecordToBuffer(SHORT_TERM_FILE, record, shortIndex, SHORT_TERM_COUNT);
        shortIndex = (shortIndex + 1) % SHORT_TERM_COUNT;
        Serial.println("Short buffer saved");

        // Каждые 6 часов — среднесрочный
        if (timeInfo->tm_hour%6 == 0) {
            writeRecordToBuffer(MID_TERM_FILE, record, midIndex, MID_TERM_COUNT);
            midIndex = (midIndex + 1) % MID_TERM_COUNT;
            Serial.println("Mid buffer saved");
        }

        // Каждые 24 часа — долгосрочный
        if (timeInfo->tm_hour == 12) {
            writeRecordToBuffer(LONG_TERM_FILE, record, longIndex, LONG_TERM_COUNT);
            longIndex = (longIndex + 1) % LONG_TERM_COUNT;
            Serial.println("Long buffer saved");
        }
    }
}

//-----------------------------------------------------------------------------

void  printBuffer(const char* filename, uint16_t count) {
    File file = LittleFS.open(filename, FILE_READ);
    if (file) {
        Serial.println("Print file "+(String)filename);
        for (int i = 0; i < count; i++) {
            Serial.print((String)i+") ");
            SensorRecord r;
            size_t rcvd = file.read((uint8_t*)&r, sizeof(SensorRecord));
            // if (r.time != 0) {
                Serial.printf("Time: %lu | T: %.1f | H: %.1f | P: %.1f  %s\n", r.time, r.temperature, r.humidity, r.pressure, (rcvd!=sizeof(SensorRecord))?"":"!!!");
            // }
        }
        file.close();
    }
}

//-----------------------------------------------------------------------------
