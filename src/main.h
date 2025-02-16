#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include "LittleFS.h"
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include "web.h"
#include "ota.h"

typedef enum display_orientation_en {
    DISPLAY_ORIENTATION_0 = 0,
    DISPLAY_ORIENTATION_CW90,
    DISPLAY_ORIENTATION_CW180,
    DISPLAY_ORIENTATION_CCW90
} display_orientation_t;

// Set LED GPIO
#define BLUE_LED_PIN    2

extern Preferences prefs;

//Variables to save values from HTML form
extern long timeOffset;

extern bool restart;

extern String timeRead;

// Stores LED state
extern String ledState;

extern display_orientation_t orientation;

#endif // __MAIN_H__
