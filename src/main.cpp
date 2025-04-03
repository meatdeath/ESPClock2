#include "main.h"
#include <TaskScheduler.h>


Scheduler runner;
DS1307 ds1307;  // I2C
hw_timer_t *ms_timer = NULL;

//Variables to save values from HTML form
long timeOffset = 0;
String timeRead = "";
bool show_ntp_time = true;
bool temperature_in_c = true;

// String ledState;// Stores LED state
bool restart = false;

enum state_en
{
    STATE_TIME = 0,
    STATE_TEMPERATURE,
    STATE_PRESSURE,
    STATE_MAX
};
uint8_t state = STATE_TIME;

uint8_t lower_intencity = 1;
uint8_t high_intencity = 6;
uint16_t higher_light = 100;
uint16_t lower_light = 3000;

volatile bool redraw_screen = true;
volatile bool colon_visible = true;
volatile bool ds1307_sqw_failure = false;

volatile bool telemetry_shown = false;
volatile uint16_t telemetry_counter = 0;


void ds1307SqwFailCallback();
Task ds1307FailCkeck(2000, 1, ds1307SqwFailCallback, &runner, false);

#define TELEMETRY_SHOW_MS       2000
#define TELEMETRY_SHOW_TIMEOUT  ((telemetry_counter==TELEMETRY_SHOW_MS)?true:false)

void CorrectDisplayBrightness();
void stateTime();

// ****************************************************************************

void IRAM_ATTR onMsTimerIsr()
{
    if (telemetry_shown && telemetry_counter<TELEMETRY_SHOW_MS) telemetry_counter++;
    onButtonMsTimerIsr();
}

//-----------------------------------------------------------------------------

void IRAM_ATTR ds1307SqwIsr()
{
    redraw_screen = true;
    colon_visible = !colon_visible; // digitalRead(DS1307_SQW_PIN)?true:false;
    ds1307_sqw_failure = false;
    ds1307FailCkeck.disable();
    ds1307FailCkeck.enable();
}

// ----------------------------------------------------------------------------

void ds1307SqwFailCallback()
{
    ds1307_sqw_failure = true;
}

// ----------------------------------------------------------------------------

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);

    pinMode(BUZER_PIN, OUTPUT);
    BEEP(50);

    // Init file system
    if (LittleFS.begin()) Serial.println("LittleFS mounted successfully");
    else Serial.println("An error has occurred while mounting LittleFS");

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

    buttonInit();

    ms_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(ms_timer, &onMsTimerIsr, true);
    timerAlarmWrite(ms_timer, 1000, true);

    // Init preferencies
    prefs.begin("setup");

    ReadSettings();

    delay(1000);
    display_VersionString();
    delay(1000);
    display_StartingString();

    Serial.print("SSID: " + ssid);
    Serial.print("PASS: " + pass);

    // Connect to Wi-Fi network with SSID and password
    if(initWiFi()) 
    {  
        accessPoint = false;
        mdns_on = MDNS.begin(mdns_name);
        if(!mdns_on) Serial.println("Error setting up MDNS responder!");
        else Serial.printf("mDNS responder started: %s.local\n", mdns_name);
        initConnectedServerEndpoints();
        BEEP(50);
    }
    else 
    {
        accessPoint = true;
        getNetworks(); // Create list of SSID
        Serial.println("Setting AP (Access Point)");
        WiFi.softAP("ESPCLOCK-AP", NULL); // NULL sets an open Access Point
        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: " + IP.toString()); 
        initDisconnectedServerEndpoints();
        BEEPS(3,50,200);
    }
    
    // Init ElegantOTA 
    ElegantOTA.begin(&server);
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

// ----------------------------------------------------------------------------

void loop()
{
    server.handleClient();
    ElegantOTA.loop();
    runner.execute();       
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
        delay(3000);
        ESP.restart();
    }

    CorrectDisplayBrightness();

    button_event_t b_event = getButtonEvent();

    switch (b_event)
    {
        case BUTTON_EVENT_PRESSED:
            BEEP(25);
            if (state < STATE_MAX) state++; else state = STATE_TIME;
            telemetry_shown = false;
            telemetry_counter = 0;
            break;
        case BUTTON_EVENT_LONG_PRESS:
            BEEPS(2,25,200);
            ResetSettings();
            break;
        case BUTTON_EVENT_LONG_LONG_PRESS:
            BEEPS(3,25,200);
            prefs.putString("ssid", "");
            prefs.putString("pass", "");
            restart = true;
            Serial.println("Reset WIFI settings. ESP will restart, connect to AP espclock.local and use 192.168.4.1 to manage WiFi connections");
            break;
        case BUTTON_EVENT_EXTRA_LONG_PRESS:
            BEEPS(4,25,200); 
            prefs.clear();
            restart = true;
            Serial.println("Reset WIFI settings. ESP will restart, connect to AP espclock.local and use 192.168.4.1 to manage WiFi connections");
            break;
    }

    switch(state)
    {
        case STATE_TIME:
            stateTime();
            break;

        case STATE_TEMPERATURE:
            if (!telemetry_shown)
            {
                telemetry_shown = true;
                if (telemetry.valid)
                {
                    display_Temperature((int)telemetry.temperature);
                }
                else
                {
                    Serial.println("BMP280 measurements are not available");
                    display_Invalid();
                    delay(500);
                    state = STATE_TIME;
                }
            }
            else
            {
                if (TELEMETRY_SHOW_TIMEOUT)
                {
                    telemetry_shown = false;
                    telemetry_counter = 0;
                    state = STATE_TIME;
                }
            }
            break;

        case STATE_PRESSURE:
            if (!telemetry_shown)
            {
                telemetry_shown = true;
                display_Pressure((int)(telemetry.pressure/133.322));
            }
            else
            {
                if (TELEMETRY_SHOW_TIMEOUT)
                {
                    telemetry_shown = false;
                    telemetry_counter = 0;
                    state = STATE_TIME;
                }
            }
            break;
            
        case STATE_MAX:
            state = STATE_TIME;
    }    
}

void CorrectDisplayBrightness()
{
    static int current_intensity = -2;
    // 3.3-3.2V -> 4095(4000)
    uint16_t lightValue = constrain(analogRead(LIGHT_SENS_PIN), higher_light, lower_light);

    
    uint8_t measured_intensity = map(
        lightValue, 
        higher_light, lower_light, 
        high_intencity + (high_intencity-lower_intencity-1), lower_intencity);
    
    if (abs(measured_intensity-current_intensity) > 2)
    {
        current_intensity = measured_intensity;
        measured_intensity = map(lightValue, higher_light, lower_light, high_intencity, lower_intencity);

        display_SetIntensity(measured_intensity);
    }
}

// ----------------------------------------------------------------------------

void stateTime()
{    
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
}

// ----------------------------------------------------------------------------
