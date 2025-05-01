#include "main.h"
#include <TaskScheduler.h>

//-----------------------------------------------------------------------------

enum state_en
{
    STATE_TIME = 0,
    STATE_TEMPERATURE,
    STATE_PRESSURE,
    STATE_MAX
};
uint8_t state = STATE_TIME;

Scheduler runner;
DS1307 ds1307;  // I2C
hw_timer_t *ms_timer = NULL;
String setupTimezone = "Autodetect";
bool timezoneDetected = false;
String timeRead = "";
bool restart = false;
volatile bool redraw_screen = true;
volatile bool colon_visible = true;
volatile bool ds1307_sqw_failure = false;
volatile bool telemetry_shown = false;
volatile uint16_t telemetry_counter = 0;

// Variables to save values from HTML form
bool show_ntp_time = true;
String temperature_units = "C";
String pressure_units = "mm";
String detectedTimezone = "";
uint8_t lower_intencity = 1;
uint8_t high_intencity = 6;
uint16_t higher_light = 100;
uint16_t lower_light = 3000;


void ds1307SqwFailCallback();
Task ds1307FailCkeck(2000, 1, ds1307SqwFailCallback, &runner, false);

#define TELEMETRY_SHOW_MS       2000
#define TELEMETRY_SHOW_TIMEOUT  ((telemetry_counter==TELEMETRY_SHOW_MS)?true:false)

void timezoneDetect();
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
    colon_visible = !colon_visible;
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

    // Enable SQW output
    Wire.beginTransmission(0x68);  // DS3231 I2C address
    Wire.write(0x0E);              // Control register
    Wire.write(0b00000000);        // INTCN=0 (bit 2), RS2:RS1 = 00 => 1Hz
    Wire.endTransmission();

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

    Serial.println("SSID: " + ssid);
    Serial.println("PASS: " + pass);

    timezoneInit();
    // Connect to Wi-Fi network with SSID and password
    if(initWiFi()) 
    {  
        accessPoint = false;
        mdns_on = MDNS.begin(mdns_name);
        if(!mdns_on) Serial.println("Error setting up MDNS responder!");
        else Serial.printf("mDNS responder started: %s.local\n", mdns_name);
        initConnectedServerEndpoints();
        BEEP(50);
        timezoneDetect();

        if (setupTimezone == "Autodetect") {
            if (timezoneDetected) {
                timezoneSetup(detectedTimezone);
            }
        } else {
            timezoneSetup(setupTimezone);
        }
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
    
    initTelemetryLog();
    
    Serial.println("Setup done.");
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
                display_Pressure(telemetry.pressure);
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

// ----------------------------------------------------------------------------

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

void showDs1307Time() {
    ds1307.getTime();

    Serial.printf("Corrected DS1307 time: %02d:%02d:%02d\n", 
        ds1307.hour,
        ds1307.minute,
        ds1307.second);

    display_Time(
        ds1307.hour, 
        ds1307.minute,
        ds1307.second, 
        ds1307_sqw_failure?true:colon_visible);
}

// *************************************************************************

void timezoneDetect() {
    HTTPClient http;
    http.begin("http://worldtimeapi.org/api/ip");
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String payload = http.getString();
        Serial.println("Ответ от сервера:");
        Serial.println(payload);

        // Разбираем JSON
        JSONVar json = JSON.parse(payload);

        if (JSON.typeof(json) == "undefined") {
            Serial.println("Ошибка парсинга JSON");
            return;
        }
    
        detectedTimezone = (const char*)json["timezone"];
        Serial.print("Временная зона: ");
        Serial.print(detectedTimezone);
        Serial.print(" | ");
        timezoneDetected = true;

        // Получаем текущее время
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
        time_t now = time(nullptr);
        int retries = 0;
        while (now < 100000 && retries < 20) {
            delay(500);
            Serial.print(".");
            now = time(nullptr);
            retries++;
        }
        Serial.println();
        if (now < 100000) {
            Serial.println("Не удалось синхронизировать время.");
            return;
        } 
       
    } else {
        Serial.printf("Ошибка HTTP-запроса: %d\n", httpCode);
    }
}

// ----------------------------------------------------------------------------

void timezoneSetup(String timezone) {
    prefs.putString("timezone", timezone);

    char szTimezone[64];
    strncpy(szTimezone, getPosixTZ(timezone).c_str(), 64);
        
    // Ставим временную зону
    Serial.println(szTimezone);
    setenv("TZ", szTimezone, 1);
    tzset();

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("Локальное время: ");
    Serial.println(buffer);
}

// ----------------------------------------------------------------------------

void stateTime()
{    
    if (accessPoint)
    {
        Serial.println("Run as a wifi server");
        if (redraw_screen)
        {
            redraw_screen = false;
            showDs1307Time();
        }
    }
    else
    {
        unsigned long corrected_epoch_time, epoch_time;
        timeClient.update();
        bool ntp_time_valid = timeClient.isTimeSet();
        
        //Serial.printf("Run as a wifi client: %d %d\n", ntp_time_valid, show_ntp_time);
        if (ntp_time_valid && show_ntp_time)
        {
            // Get epoch time from NTP
            epoch_time = timeClient.getEpochTime();
            
            // Extract day/time info from epoch time
            struct tm *timeinfo;
            timeinfo = localtime((time_t*)&epoch_time);

            // Create string to return via web
            char time_str[20] ="";
            snprintf(time_str, 20, "%02d:%02d:%02d", 
                timeinfo->tm_hour, 
                timeinfo->tm_min, 
                timeinfo->tm_sec);
            timeRead = String(time_str);

            // Check if screen needs to be updated
            if (redraw_screen || (ds1307_sqw_failure && timeinfo->tm_sec == 0))
            {
                redraw_screen = false;

                // Serial.printf("Show time: %s | telemetry valid: %s\n", time_str, telemetry.valid?"yes":"no");

                static int last_log_hour = -1;
                static int last_log_minute = -1;

                if (telemetry.valid && 
                    // correctedTimeinfo->tm_sec == 0 && 
                    // last_log_minute != correctedTimeinfo->tm_min
                    last_log_hour != timeinfo->tm_hour &&
                    timeinfo->tm_min == 0
                ) {
                    last_log_hour = timeinfo->tm_hour;
                    last_log_minute = timeinfo->tm_min;
                    logTelemetry(epoch_time, telemetry.temperature, telemetry.pressure, telemetry.humidity);
                }

                display_Time( 
                    timeinfo->tm_hour, 
                    timeinfo->tm_min, 
                    timeinfo->tm_sec, 
                    colon_visible);

                if (timeinfo->tm_sec == 30) {
                    // Constant defines maximum time difference between DS1307 and NTP
                    const unsigned long diif_limit = 5;
                    
                    // Get time from DS1307
                    ds1307.getTime();

                    // Serial.printf("DS1307 time read: %4d-%02d-%02d %02d:%02d:%02d\n",
                    //     ds1307.year, ds1307.month, ds1307.dayOfMonth,
                    //     ds1307.hour, ds1307.minute, ds1307.second);

                    // Fill time info structure to convert it into epoch time
                    struct tm ds1307Timeinfo;
                    ds1307Timeinfo.tm_year  = ds1307.year + 2000 - 1900;  // In info year starts from 1900, in ds1307 year starts from 2000
                    ds1307Timeinfo.tm_mon   = ds1307.month - 1;     // Месяц (0 = Январь, 3 = Апрель)
                    ds1307Timeinfo.tm_mday  = ds1307.dayOfMonth;   // День месяца
                    ds1307Timeinfo.tm_hour  = ds1307.hour;         // Час (24ч формат)
                    ds1307Timeinfo.tm_min   = ds1307.minute;        // Минуты
                    ds1307Timeinfo.tm_sec   = ds1307.second;        // Секунды
                    ds1307Timeinfo.tm_isdst = timeinfo->tm_isdst;                  // Летнее время (0 = нет)
                
                    // Converting DS1307 time info into epoch time 
                    unsigned long ds1307_epoch_time = (unsigned long)mktime(&ds1307Timeinfo);

                    // // Print time/timestamps from DS1307 and NTP
                    // Serial.printf("NTP date/time: %lu - %4d-%02d-%02d %02d:%02d:%02d\n",
                    //     epoch_time,
                    //     timeinfo->tm_year + 1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
                    //     timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                    // Serial.printf("DS1307 time:   %lu - %4d-%02d-%02d %02d:%02d:%02d\n",
                    //     ds1307_epoch_time,
                    //     ds1307Timeinfo.tm_year + 1900, ds1307Timeinfo.tm_mon+1, ds1307Timeinfo.tm_mday,
                    //     ds1307Timeinfo.tm_hour, ds1307Timeinfo.tm_min, ds1307Timeinfo.tm_sec);

                    // Serial.printf("Epochs: %lu %lu\n", epoch_time, ds1307_epoch_time);

                    // Get time difference in seconds
                    uint32_t second_diff = 0;
                    if (epoch_time > ds1307_epoch_time)
                        second_diff = epoch_time - ds1307_epoch_time;
                    else
                        second_diff = ds1307_epoch_time - epoch_time;

                    // Check if time difference doesn;t exceed the limit
                    if (second_diff > diif_limit)
                    {                    
                        // Print time/timestamps from DS1307 and NTP
                        Serial.printf("Time difference: %lus\n", second_diff);

                        // Send syncronization date & time to DS1307
                        ds1307.fillByYMD(timeinfo->tm_year + 1900, timeinfo->tm_mon+1, timeinfo->tm_mday);
                        ds1307.fillByHMS(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

                        Serial.printf("Store time: %4d-%02d-%02d %02d:%02d:%02d\n",
                            timeinfo->tm_year + 1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
                            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

                        // Apply DS1307 setup
                        ds1307.setTime();

                        Serial.println("DS1307 time syncronized with the time from NTP.");
                    }
                }
            }
        } else {
            if (redraw_screen) {
                redraw_screen = false;
                showDs1307Time();
            } 
            else if (ds1307_sqw_failure) {
                Serial.println("SQW fail");
                display_Invalid();
                Wire.beginTransmission(0x68);  // DS3231 I2C address
                Wire.write(0x0E);              // Control register
                Wire.write(0b00000000);        // INTCN=0 (bit 2), RS2:RS1 = 00 => 1Hz
                Wire.endTransmission();
                ds1307FailCkeck.enable();
            }
        }
    }
}

// ----------------------------------------------------------------------------
