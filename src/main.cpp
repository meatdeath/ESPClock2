#include "main.h"
#include <TaskScheduler.h>

#define DS1307_SQW_PIN      13

#define BUZER_PIN           14
#define BUZER_ON()          digitalWrite(BUZER_PIN, LOW)
#define BUZER_OFF()         digitalWrite(BUZER_PIN, HIGH)
#define BEEP(DELAY_MS)      do{BUZER_ON();delay(DELAY_MS);BUZER_OFF();}while(0)

Scheduler runner;

Preferences prefs;
DS1307 ds1307;  // I2C
Adafruit_BMP280 bmp280; // I2C

//Variables to save values from HTML form
long timeOffset = 0;
String timeRead = "";
bool show_ntp_time = true;
// Stores LED state
String ledState;
bool restart = false;

struct telemetry_st{
    bool valid;
    float temperature;
    float pressure;
} telemetry = {false, -273, 0};

uint8_t lower_intencity = 1;
uint8_t high_intencity = 6;
uint16_t higher_light = 100;
uint16_t lower_light = 3000;

volatile bool button_pressed = false;
volatile bool redraw_screen = true;
volatile bool colon_visible = true;
volatile bool ds1307_sqw_failure = false;

#ifdef ESP32
void browseService(const char * service, const char * proto);
#endif

void bmp280Callback();
Task bmp280Task(5000, TASK_FOREVER, &bmp280Callback, &runner, false);
void ds1307SqwFailCallback();
Task ds1307FailCkeck(2000, 1, ds1307SqwFailCallback, &runner, false);

void IRAM_ATTR buttonIsr() 
{
    button_pressed = true;
}

void IRAM_ATTR ds1307SqwIsr()
{
    redraw_screen = true;
    colon_visible = !colon_visible; // digitalRead(DS1307_SQW_PIN)?true:false;
    ds1307_sqw_failure = false;
    ds1307FailCkeck.disable();
    ds1307FailCkeck.enable();
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

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);

    pinMode(BUZER_PIN, OUTPUT);
    BEEP(200);

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

    pinMode(DS1307_SQW_PIN, INPUT);
	attachInterrupt(DS1307_SQW_PIN, ds1307SqwIsr, CHANGE);

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
    bmp280Init();

	pinMode(BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(BUTTON_PIN, buttonIsr, FALLING);

    // Init preferencies
    prefs.begin("setup");

    // Load values saved in preferencies
    Serial.print("Reading preferencies... ");

    ssid            = prefs.getString("ssid", "");
    pass            = prefs.getString("pass", "");
    language        = prefs.getString("language", "en");
    
    lower_intencity = prefs.getInt("lower_intencity", 1);
    high_intencity  = prefs.getInt("high_intencity", 8);
    higher_light    = prefs.getInt("higher_light", 200);
    lower_light     = prefs.getInt("lower_light", 3000);

    matrix_order        = prefs.getString("matrix_order", "reverse");
    matrix_orientation  = (matrix_orientation_t)prefs.getInt("orientation", MATRIX_ORIENTATION_0);

    timeOffset                  = prefs.getInt("timeOffset", 0);
    time_format                 = prefs.getUInt("timeFormat", TIME_FORMAT_24H);
    display_show_leading_zero   = prefs.getBool("leading_zero", true);
    show_ntp_time               = prefs.getBool("show_ntp_time", true);

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

        initConnectedServerEndpoints();
        BEEP(50);
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
        
        initDisconnectedServerEndpoints();
        
        BEEP(50);
        delay(200);
        BEEP(50);
        delay(200);
        BEEP(50);
    }

    ElegantOTA.begin(&server);    // Start ElegantOTA
    
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    server.begin();
    Serial.println("HTTP server started");
    
    runner.startNow();  // set point-in-time for scheduling start
    bmp280Task.enable();
    ds1307FailCkeck.enable();
    
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

void bmp280Callback()
{
    Serial.println("Time to get measurements from bmp280...");
    if (bmp280.takeForcedMeasurement())
    {
        telemetry.temperature = bmp280.readTemperature();
        telemetry.pressure = bmp280.readPressure();
        Serial.printf("Temperature: %2.1f*C\n", telemetry.temperature);
        Serial.printf("Pressure: %.1fPa = %.2fmmHg\n", telemetry.pressure, telemetry.pressure/133.322);
        telemetry.valid = true;
    }
    else
    {
        Serial.println("ERROR! Failed to get measurements from BMP280! Reinitialization...");
        bmp280Init();
    }
}

void ds1307SqwFailCallback()
{
    ds1307_sqw_failure = true;
}

void SecToTime(uint32_t time_sec, uint8_t &hour, uint8_t &minute, uint8_t &second)
{
    hour = (time_sec/3600)%24;
    minute = (time_sec/60)%60;
    second = time_sec%60;
}

// ----------------------------------------------------------------------------

void handleLoopFuncs()
{
    server.handleClient();
    ElegantOTA.loop();
    runner.execute();
}

void loop()
{
    handleLoopFuncs();
	if (isButtonPressed()) 
    {   
        BEEP(50);
        if (telemetry.valid)
        {
            display_Temperature((int)telemetry.temperature);
            for (int i = 0; i < 2000; i++) {
                delay(1);
                if (isButtonPressed()) 
                {
                    BEEP(50);
                    display_Pressure((int)(telemetry.pressure/133.322));
                    for (i = 0; i < 2000; i++) {
                        delay(1);
                        if (isButtonPressed()) 
                        {
                            BEEP(50);
                            i = 2000;
                        }
                    }
                }
            }
        }
        else
        {
            Serial.println("BMP280 measurements are not available");
            display_Invalid();
            delay(500);
        }
    }

    // 3.3-3.2V -> 4095(4000)
    uint16_t lightValue = constrain(analogRead(LIGHT_SENS_PIN), higher_light, lower_light);
    uint8_t measured_intensity = map(lightValue, higher_light, lower_light, high_intencity, lower_intencity);

    display_SetIntensity(measured_intensity);
    
    if (accessPoint)
    {
        if (redraw_screen)
        {
            redraw_screen = false;
            ds1307.getTime();
            
            // +24 hours required to avoid overflow on correction
            unsigned long corrected_time = (24+ds1307.hour) * 3600 + ds1307.minute * 60 + ds1307.second + timeOffset;
            uint8_t corrected_hour, corrected_minute, corrected_second;    
            SecToTime(corrected_time, corrected_hour, corrected_minute, corrected_second);

            char time_str[20] ="";
            snprintf(time_str, 20, "%02d:%02d:%02d", corrected_hour, corrected_minute, corrected_second);
            Serial.printf("Corrected DS1307 time: %s\n", time_str);

            display_Time(corrected_hour, corrected_minute, corrected_second, colon_visible);
        }
        else if (ds1307_sqw_failure)
        {
            display_Invalid();
        }
    }
    else
    {
        uint8_t corrected_hour, corrected_minute, corrected_second;
        uint8_t hour, minute, second;
        unsigned long corrected_time, epoch_time;
        bool ntp_time_valid = timeClient.update();

        if (ntp_time_valid && show_ntp_time)
        {
            epoch_time = timeClient.getEpochTime();
            corrected_time = epoch_time + timeOffset;
            
            SecToTime(corrected_time, corrected_hour, corrected_minute, corrected_second);
            SecToTime(epoch_time, hour, minute, second);

            char time_str[20] ="";
            snprintf(time_str, 20, "%02d:%02d:%02d", corrected_hour, corrected_minute, corrected_second);
            timeRead = String(time_str);

            if (redraw_screen || (ds1307_sqw_failure && corrected_second == 0))
            {
                redraw_screen = false;
                Serial.printf("Show time: %s\n", time_str);

                display_Time(corrected_hour, corrected_minute, corrected_second, colon_visible);
                
                ds1307.getTime();

                const unsigned long seconds_in_day = 24*60*60;
                const unsigned long diif_limit = 5;

                unsigned long epoch_day_seconds = epoch_time%seconds_in_day;
                unsigned long ds1307_day_seconds = ds1307.hour*60*60 + ds1307.minute*60 + ds1307.second;

                // if seconds not around overflow
                if ((epoch_day_seconds > diif_limit && ds1307_day_seconds < (seconds_in_day-diif_limit)) ||
                    (ds1307_day_seconds > diif_limit && epoch_day_seconds < (seconds_in_day-diif_limit)) )
                {
                    uint8_t second_diff = 0;
                    if (epoch_day_seconds > ds1307_day_seconds)
                        second_diff = epoch_day_seconds - ds1307_day_seconds;
                    else
                        second_diff = ds1307_day_seconds - epoch_day_seconds;

                    if (second_diff > diif_limit)
                    {
                        Serial.printf("DS1307 time: %02d:%02d:%02d\n", ds1307.hour, ds1307.minute, ds1307.second);
                        delay(500);
                        ds1307.fillByHMS(hour, minute, second+1);
                        ds1307.setTime();
                        Serial.printf("Syncronized DS1307 with NTP time: %s\n", time_str);
                    }
                }
            }
        }
        else
        {
            if (redraw_screen)
            {
                redraw_screen = false;

                ds1307.getTime();

                // +24 hours required to avoid overflow on correction
                corrected_time = (24+ds1307.hour) * 3600 + ds1307.minute * 60 + ds1307.second + timeOffset;
                SecToTime(corrected_time, corrected_hour, corrected_minute, corrected_second);
                
                char time_str[20] ="";
                snprintf(time_str, 20, "%02d:%02d:%02d", corrected_hour, corrected_minute, corrected_second);
                timeRead = String(time_str);

                display_Time(corrected_hour, corrected_minute, corrected_second, colon_visible);
            }
            else if (ds1307_sqw_failure)
            {
                display_Invalid();
            }
        }
    }
    
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
