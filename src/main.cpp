#include "main.h"

Preferences prefs;
//Variables to save values from HTML form
long timeOffset = 0;
String timeRead = "";
// Stores LED state
String ledState;
bool restart = false;

#ifdef ESP32
void browseService(const char * service, const char * proto);
#endif

// ****************************************************************************

// Initialize LittleFS
void initFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("An error has occurred while mounting LittleFS");
    }
    else
    {
        Serial.println("LittleFS mounted successfully");
    }
}

// ----------------------------------------------------------------------------

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);

    initFS();

    // Set GPIO 2 as an OUTPUT
    pinMode(BLUE_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, HIGH);

    Serial.print("Display init... ");
    display_Init();
    Serial.println("done");

    // Load values saved in LittleFS
    Serial.print("Reading preferencies... ");
    prefs.begin("setup");
    timeOffset = prefs.getInt("timeOffset", 0);
    ssid = prefs.getString("ssid","");
    pass = prefs.getString("pass","");
    language = prefs.getString("language","en");
    Serial.println("done");

    display_ClockString();
    delay(500);
    display_VersionString();
    delay(500);
    display_StartingString();

    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("PASS:");
    Serial.println(pass);

    if(initWiFi()) 
    {  
        accessPoint = false;
        mdns_on = MDNS.begin(mdns_name);

        if(!mdns_on)
            Serial.println("Error setting up MDNS responder!");
        else
            Serial.printf("mDNS responder started: %s.local\n", mdns_name);

        // Route for root / web page
        server.on("/", handleRoot);
        server.on("/reset", handleReset);
        server.on("/on", handleLedOn);
        server.on("/off", handleLedOff);
        server.on("/style.css", handleCss);
        server.on("/index.js", handleJs);
        server.on("/timeread", handleTime);
        server.on("/timeoffset", handleTimeOffset);
        server.on("/orientationRequest", handleOrientationRequest);
    }
    else 
    {
        accessPoint = true;

        // Create list of SSID
        getNetworks();

        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
        // NULL sets an open Access Point
        WiFi.softAP("ESPCLOCK-AP", NULL);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 

        // Web Server Root URL
        server.on("/", handleWiFiManager);
        server.on("/style.css", handleCss);
        server.on("/wifimanager.js", handleWifiManagerJs);
    }

    ElegantOTA.begin(&server);    // Start ElegantOTA
    
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    server.begin();
    Serial.println("HTTP server started");
    
    timeClient.begin();
}

// ----------------------------------------------------------------------------

void loop()
{
    server.handleClient();
    ElegantOTA.loop();
    
    if (accessPoint == false)
    {
        timeClient.update();
        static String old_time = "";
        //timeRead = timeClient.getFormattedTime();
        unsigned long epoch_time = timeClient.getEpochTime() + timeOffset;
        char time_str[20] ="";
        uint8_t hours = (epoch_time/3600)%24;
        uint8_t minutes = (epoch_time/60)%60;
        uint8_t seconds = epoch_time%60;
        snprintf(time_str, 20, "%02d:%02d:%02d", hours, minutes, seconds);
        timeRead = String(time_str);
        if (timeRead != old_time)
        {
            Serial.println(timeRead);
            old_time = timeRead;
            display_Time(hours, minutes, seconds, DISPLAY_FORMAT_24H);
        }
    }
    //ElegantOTA.loop();
    if(mdns_on) 
    {
        #ifdef ESP8266
        MDNS.update();
        #else
        //browseService("http", "tcp");
        #endif
    }
    if (restart)
    {
        delay(5000);
        ESP.restart();
    }
}

#ifdef ESP32
void browseService(const char * service, const char * proto)
{
    Serial.printf("Browsing for service _%s._%s.local. ... ", service, proto);
    int n = MDNS.queryService(service, proto);
    if (n == 0) {
        Serial.println("no services found");
    } else {
        Serial.print(n);
        Serial.println(" service(s) found");
        for (int i = 0; i < n; ++i) {
            // Print details for each service found
            Serial.print("  ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(MDNS.hostname(i));
            Serial.print(" (");
            Serial.print(MDNS.IP(i));
            Serial.print(":");
            Serial.print(MDNS.port(i));
            Serial.println(")");
        }
    }
    Serial.println();
}
#endif
