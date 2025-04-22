#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#include <WiFiClient.h>
#include <ElegantOTA.h>
#include "LittleFS.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include <SPI.h>
#include "MATRIX7219.h"
#include <Wire.h>
#include "DS1307.h"
#include <Adafruit_BMP280.h>
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskSchedulerDeclarations.h>
#include <Arduino_JSON.h>

#include "web.h"
#include "ota.h"
#include "display.h"
#include "button.h"
#include "buzer.h"
#include "bmp.h"
#include "tools.h"
#include "telemetry_log.h"

#define VERSION_MAJOR   2
#define VERSION_MINOR   0

// Set LED GPIO
#ifdef ESP8266
#define BLUE_LED_PIN    2
#define I2C_SCL_PIN     1
#define I2C_SDA_PIN     2
#define BUTTON_PIN      0
#define LIGHT_SENS_PIN  A0
#else
#define BLUE_LED_PIN    22
#define I2C_SCL_PIN     16
#define I2C_SDA_PIN     17
#define BUTTON_PIN      25
#define LIGHT_SENS_PIN  36
#define DS1307_SQW_PIN  13
#define BUZER_PIN       14
#endif

#define DS1307_I2C_ADDR 0x68
#define BMP280_I2C_ADDR 0x76

#define LIGHT_MAX       4000

extern Preferences prefs;

//Variables to save values from HTML form
extern long timeOffset;
extern bool restart;
extern String timeRead;
// Stores LED state
// extern String ledState;
extern uint16_t higher_light;
extern uint16_t lower_light;
extern uint8_t lower_intencity;
extern uint8_t high_intencity;
extern bool show_ntp_time;
extern String temperature_units;
extern String pressure_units;
extern hw_timer_t *ms_timer;
extern Scheduler runner;

void ResetSettings();

#endif // __MAIN_H__
