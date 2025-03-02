#include "main.h"

Preferences prefs;
DS1307 ds1307;  // I2C
Adafruit_BMP280 bmp280; // I2C

//Variables to save values from HTML form
long timeOffset = 0;
String timeRead = "";
// Stores LED state
String ledState;
bool restart = false;
float temperature = -273.0;
float pressure = 0;

volatile bool button_pressed = false;

#ifdef ESP32
void browseService(const char * service, const char * proto);
#endif

void IRAM_ATTR buttonIsr() 
{
    button_pressed = true;
}

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
    display_ClockString();
    Serial.println("done");

    Serial.print("Init DS1307... ");
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
	ds1307.begin();

    //Wire.begin( I2C_SDA_PIN, I2C_SCL_PIN);
    // for (byte address = 0x08; address < 0x7F; address++) {  // Scan addresses from 0x08 to 0x7F
    //     Wire.beginTransmission(address); // Start transmission to the address
    //     if (Wire.endTransmission() == 0) { // Check if transmission was successful (device responded)
    //       Serial.print("Found address: 0x"); 
    //       Serial.println(address, HEX); // Print the found address in hexadecimal format
    //     }
    //     delay(5); // Small delay between scans    
    //   }

	ds1307.getTime();
    Serial.println("done");

    Serial.printf("Time from DS1307: %02d:%02d:%02d\n", ds1307.hour, ds1307.minute, ds1307.second);

    Serial.print("Init BMP280... ");
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

    
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(BUTTON_PIN, buttonIsr, FALLING);

    // Load values saved in LittleFS
    Serial.print("Reading preferencies... ");
    prefs.begin("setup");
    timeOffset = prefs.getInt("timeOffset", 0);
    ssid = prefs.getString("ssid","");
    pass = prefs.getString("pass","");
    language = prefs.getString("language","en");
    Serial.println("done");

    delay(1000);
    display_VersionString();
    delay(1000);
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

bool isButtonPressed(void)
{
    if (button_pressed) 
    {
        button_pressed = false;
        for( int i = 0; i < 100; i++)
        {
            delay(1);
            if (digitalRead(BUTTON_PIN) != 0) 
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

void handleLoopFuncs()
{
    server.handleClient();
    ElegantOTA.loop();
}

void loop()
{
    handleLoopFuncs();
	if (isButtonPressed()) 
    {   
        if (bmp280.takeForcedMeasurement()) 
        {
            temperature = bmp280.readTemperature();
            pressure = bmp280.readPressure();
            Serial.printf("Temperature: %2.1f*C\n", temperature);
            Serial.printf("Pressure: %.1fPa = %.2fmmHg\n", pressure, pressure/133.322);
        }
        if (temperature != -273.0)
        {
            display_Temperature((int)temperature);
            for (int i = 0; i < 2000; i++) {
                delay(1);
                if (isButtonPressed()) 
                {
                    if (pressure != 0)
                    {
                        display_Pressure((int)(pressure/133.322));
                        for (i = 0; i < 2000; i++) {
                            delay(1);
                            if (isButtonPressed()) 
                            {
                                i = 2000;
                            }
                        }
                    }
                }
            }
        }
    }

    // 3.3-3.2V -> 4095
    const int lower_light = 100;
    const int higher_light = 3000;
    uint16_t lightValue = constrain(analogRead(LIGHT_SENS_PIN), lower_light, higher_light);
    uint8_t measured_intensity = map(lightValue, lower_light, higher_light, 6, 0);

    display_SetIntensity(measured_intensity);
    
    if (accessPoint)
    {
        ds1307.getTime();
        static uint8_t seconds = 60;
        if (seconds != ds1307.second)
        {
            seconds = ds1307.second;
            uint8_t hours = ds1307.hour;
            uint8_t minutes = ds1307.minute;
            
            char time_str[20] ="";
            snprintf(time_str, 20, "%02d:%02d:%02d", hours, minutes, seconds);
            Serial.printf("DS1307 time: %s\n", time_str);
            //Serial.printf("Intensity = %d (%d)\n", measured_intensity, lightValue);
            display_Time(hours, minutes, seconds);
        }
    }
    else
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
            Serial.printf("Time: %s\n", timeRead.c_str());
            //Serial.printf("Intensity = %d (%d)\n", measured_intensity, lightValue);
            old_time = timeRead;
            display_Time(hours, minutes, seconds);
            
            ds1307.getTime();
            if (hours != ds1307.hour || minutes != ds1307.minute || seconds != ds1307.second)
            {
                Serial.printf("DS1307 time: %02d:%02d:%02d\n", ds1307.hour, ds1307.minute, ds1307.second);
                delay(500);
                ds1307.fillByHMS(hours, minutes, seconds+1);
                ds1307.setTime();
                Serial.println("Sync DS1307 with NTP time");
            }
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
