#include "main.h"

unsigned long ota_progress_millis = 0;

void onOTAStart() 
{
    Serial.println("OTA update started!");
// <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) 
{
    // Log every 1 second
    if (millis() - ota_progress_millis > 1000) {
        ota_progress_millis = millis();
        Serial.printf("OTA Progress: Current = %u bytes / Final = %u bytes\n", current, final);
    }
}

void onOTAEnd(bool success) 
{
    // Log when OTA has finished
    Serial.println();
    if (success) {
        Serial.println("OTA update finished successfully!");
    } else {
        Serial.println("There was an error during OTA update!");
    }
    // <Add your own code here>
}
