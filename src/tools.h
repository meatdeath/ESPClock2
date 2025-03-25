#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <Preferences.h>

void ResetSettings();
void ReadSettings();
void SecToTime(uint32_t time_sec, uint8_t &hour, uint8_t &minute, uint8_t &second);

extern Preferences prefs;

#endif // __SETTINGS_H__