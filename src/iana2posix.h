#ifndef __IANA_TO_POSIX_H__
#define __IANA_TO_POSIX_H__

#include <Arduino.h>

#define TIMEZONE_NUM  92

void timezoneInit();
String getPosixTZ(String ianaTZ);
String getIanaTZ(int tz_index);

#endif // __IANA_TO_POSIX_H__
