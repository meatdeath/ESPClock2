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


// Set LED GPIO
#define BLUE_LED_PIN    2

extern Preferences prefs;


//Variables to save values from HTML form
extern long timeOffset;

extern bool restart;

extern String timeRead;
// Stores LED state
extern String ledState;

#endif // __MAIN_H__
