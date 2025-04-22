#include "main.h"

Adafruit_BMP280 bmp280; // I2C

telemetry_t telemetry = {.valid=false, .temperature=-273, .pressure=0, .humidity=0};

void bmp280Callback();
Task bmp280Task(5000, TASK_FOREVER, &bmp280Callback, &runner, false);


// ----------------------------------------------------------------------------

bool bmp280Init() 
{
    telemetry.valid = false;
    bool bmp_initialized = bmp280.begin(BMP280_I2C_ADDR);
    if (!bmp_initialized)
    {
        Serial.println(F("FAIL!\nCould not find a valid BMP280 sensor, check wiring or "
                          "try a different address!"));
    }
    else
    {
        /* Default settings from datasheet. */
        bmp280.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
            Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
        Serial.println("done");
    }
    return bmp_initialized;
}

// ----------------------------------------------------------------------------

void bmp280Callback()
{
    // Serial.println("Time to get measurements from bmp280...");
    if (bmp280.takeForcedMeasurement())
    {
        telemetry.temperature = bmp280.readTemperature();
        telemetry.pressure = bmp280.readPressure();
        // Serial.printf("Temperature: %2.1f*C\n", telemetry.temperature);
        // Serial.printf("Pressure: %.1fPa = %.2fmmHg\n", telemetry.pressure, telemetry.pressure/133.322);
        telemetry.valid = true;
    }
    else
    {
        Serial.println("ERROR! Failed to get measurements from BMP280! Reinitialization...");
        bmp280Init();
    }
}

// ----------------------------------------------------------------------------