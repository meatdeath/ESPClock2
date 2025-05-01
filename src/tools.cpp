#include "main.h"

Preferences prefs;

// ----------------------------------------------------------------------------

void ResetSettings()
{
    Serial.println("Reset setup to default");
    lower_intencity             = 1; 
    high_intencity              = 8; 
    higher_light                = 200; 
    lower_light                 = 3000; 
    matrix_order                = "reverse"; 
    matrix_orientation          = MATRIX_ORIENTATION_0;
    time_format                 = TIME_FORMAT_24H; 
    display_show_leading_zero   = true; 
    show_ntp_time               = true;
    temperature_units            = "C";
    pressure_units              = "mm";
    prefs.putInt("lower_intencity", 1);
    prefs.putInt("high_intencity", 8);
    prefs.putInt("higher_light", 200);
    prefs.putInt("lower_light", 3000);
    prefs.putString("matrix_order", "reverse");
    prefs.putInt("orientation", MATRIX_ORIENTATION_0);
    prefs.putUInt("timeFormat", TIME_FORMAT_24H);
    prefs.putBool("leading_zero", true);
    prefs.putBool("show_ntp_time", true);
    prefs.putString("temp_units", "C");
    prefs.putString("pressure_units", "mm");
    prefs.putString("timezone", "America/Toronto");
    
    setenv("TZ", getPosixTZ("America/Toronto").c_str(), 1);
    tzset();
}

// ----------------------------------------------------------------------------

void ReadSettings()
{
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

    time_format                 = prefs.getUInt("timeFormat", TIME_FORMAT_24H);
    display_show_leading_zero   = prefs.getBool("leading_zero", true);
    show_ntp_time               = prefs.getBool("show_ntp_time", true);
    temperature_units           = prefs.getString("temp_units", "C");
    pressure_units              = prefs.getString("pressure_units", "mm");
    setupTimezone               = prefs.getString("timezone", "America/Toronto");
    
    setenv("TZ", getPosixTZ("America/Toronto").c_str(), 1);
    tzset();

    Serial.println("done");
}

// ----------------------------------------------------------------------------

void SecToTime(uint32_t time_sec, uint8_t &hour, uint8_t &minute, uint8_t &second)
{
    hour = (time_sec/3600)%24;
    minute = (time_sec/60)%60;
    second = time_sec%60;
}

// ----------------------------------------------------------------------------
