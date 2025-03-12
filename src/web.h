#ifndef __WEB_H__
#define __WEB_H__


#define MDNS_NAME       "espclock2"
#define WAIT_WIFI_TIME  20  // 20s

#define MAX_NETWORKS 25

typedef struct {
    String ssid;
    int32_t rssi;
    String description;
}
networkInfo_t;

bool initWiFi();
void handleWiFiManager(void);
void getNetworks(void);


void handleRoot(void);
void handleReset(void);
void handleRestart(void);
void handleCss(void);
void handleJs(void);
void handleTimeOffset(void);
void handleTimeFormat(void);
void handleBrightness(void);
void handleTime(void);
void handleLedOff(void);
void handleLedOn(void);
void handleMatrix(void);
void handleWifiManagerJs(void);

extern String ssid;
extern String pass;
extern String language;

extern const char* mdns_name;

extern bool accessPoint;
extern bool mdns_on;

#ifdef ESP8266
extern ESP8266WebServer server;
#else
extern WebServer server;
#endif

//extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

#endif // __WEB_H__
