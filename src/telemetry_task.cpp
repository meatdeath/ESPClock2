#include "main.h"

Adafruit_BMP280 bmp280; // I2C
AHT10 myAHT20(AHT10_ADDRESS_0X38, AHT20_SENSOR);

telemetry_t telemetry = {.valid=false, .temperature=-273, .pressure=0, .humidity=0};

void TelemetryCallback();
Task TelemetryTask(5000, TASK_FOREVER, &TelemetryCallback, &runner, false);


// ----------------------------------------------------------------------------

bool TelemetryInit() 
{
    Serial.print("Init BMP280 on address 0x76... ");
    telemetry.valid = false;
    bool telemetryInitialized = bmp280.begin(BMP280_I2C_ADDR_76);

    if (!telemetryInitialized)
    {
        Serial.print(F("BMP280 sensor not found. \nTrying address 0x77..."));
        telemetryInitialized = bmp280.begin(BMP280_I2C_ADDR_77);
    }

    if (telemetryInitialized)
    {
        /* Default settings from datasheet. */
        bmp280.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
            Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
        Serial.println("done");
    } else {
        Serial.println(F("FAIL!\nCould not find a valid BMP280 sensor, check wiring or "
                          "try a different address!"));
    }

    if (myAHT20.begin() != true)
    {
        telemetryInitialized = false;
        Serial.println(F("AHT20 not connected or fail to load calibration coefficient"));
    }

    return telemetryInitialized;
}

// ----------------------------------------------------------------------------

void TelemetryCallback()
{
    // Serial.println("Time to get measurements from bmp280...");
    if (bmp280.takeForcedMeasurement())
    {
        telemetry.temperature = bmp280.readTemperature();
        telemetry.pressure = bmp280.readPressure();
        telemetry.humidity = myAHT20.readHumidity();
        Serial.printf("Temperature: %2.1f*C\n", telemetry.temperature);
        Serial.printf("Pressure: %.1fPa = %.2fmmHg\n", telemetry.pressure, telemetry.pressure/133.322);
        Serial.printf("Humidity: %3.0f%%\n", telemetry.humidity);
        telemetry.valid = true;
    }
    else
    {
        Serial.println("ERROR! Failed to get measurements from BMP280! Reinitialization...");
        TelemetryInit();
    }
}

// ----------------------------------------------------------------------------