#include "main.h"

Preferences prefs;
//Variables to save values from HTML form
long timeOffset = 0;
String timeRead = "";
// Stores LED state
String ledState;
bool restart = false;

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

void setup() {
    // Serial port for debugging purposes
    Serial.begin(115200);

    initFS();

    // Set GPIO 2 as an OUTPUT
    pinMode(BLUE_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, HIGH);
    
    // Load values saved in LittleFS
    prefs.begin("setup");
    timeOffset = prefs.getInt("timeOffset", 0);
    ssid = prefs.getString("ssid","");
    pass = prefs.getString("pass","");
    language = prefs.getString("language","en");
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
        server.on("/timeread", handleTime);
        server.on("/timeoffset", handleTimeOffset);
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
        snprintf(time_str, 20, "%02lu:%02lu:%02lu", (epoch_time/3600)%24, (epoch_time/60)%60, epoch_time%60);
        timeRead = String(time_str);
        if (timeRead != old_time)
        {
            Serial.println(timeRead);
            old_time = timeRead;
        }
    }
    //ElegantOTA.loop();
    if(mdns_on) 
    {
        MDNS.update();
    }
    if (restart)
    {
        delay(5000);
        ESP.restart();
    }
}
