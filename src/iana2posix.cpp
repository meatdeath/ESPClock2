#include "iana2posix.h"

typedef struct timeZone_st {
  String ianaTZ;
  String posixTZ;
} timeZone_t;

timeZone_t timeZone[TIMEZONE_NUM] = {
  { "America/Toronto",      "EST5EDT,M3.2.0,M11.1.0" },
  { "America/New_York",     "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Chicago",      "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Denver",       "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Los_Angeles",  "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Argentina/Buenos_Aires", "ART-3" },
  { "America/Sao_Paulo",    "BRT3BRST,M10.1.0,M2.3.0" },
  { "America/Mexico_City",  "CST6CDT,M4.1.0,M10.5.0" },
  { "America/Montevideo",   "UYT3UYST,M10.5.0,M3.3.0" },
  { "America/Indianapolis", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Anchorage",    "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/Houston",      "CST6CDT,M3.2.0,M11.1.0" },

  { "Europe/London",        "GMT0BST,M3.2.0,M10.5.0" },
  { "Europe/Paris",         "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Berlin",        "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Moscow",        "MSK-3" },
  { "Europe/Madrid",        "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Stockholm",     "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Oslo",          "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Zurich",        "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Prague",        "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Belgrade",      "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Skopje",        "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Podgorica",     "CET-1CEST,M3.2.0,M10.5.0" },
  { "Europe/Chisinau",      "EET-2EEST,M3.2.0,M10.5.0" },
  { "Europe/Tirane",        "CET-1CEST,M3.2.0,M10.5.0" },

  { "Asia/Tokyo",           "JST-9" },
  { "Asia/Singapore",       "SGT-8" },
  { "Asia/Kolkata",         "IST-5:30" },
  { "Asia/Hong_Kong",       "HKT-8" },
  { "Asia/Shanghai",        "CST-8" },
  { "Asia/Seoul",           "KST-9" },
  { "Asia/Manila",          "PST-8" },
  { "Asia/Dubai",           "GST-4" },
  { "Asia/Bangkok",         "ICT-7" },
  { "Asia/Kuwait",          "AST-3" },
  { "Asia/Karachi",         "PKT-5" },
  { "Asia/Baghdad",         "AST-3" },
  { "Asia/Baku",            "AZT-4" },
  { "Asia/Yerevan",         "AMT-4" },
  { "Asia/Almaty",          "ALMT-6" },
  { "Asia/Tashkent",        "UZT-5" },
  { "Asia/Tehran",          "IRST-3:30,IRDT-4:30,M3.0.0,M9.0.0" },
  { "Asia/Jakarta",         "WIB-7" },
  { "Asia/Chongqing",       "CST-8" },
  { "Asia/Kuala_Lumpur",    "MYT-8" },
  { "Asia/Vientiane",       "ICT-7" },
  { "Asia/Taipei",          "CST-8" },
  { "Asia/Manama",          "AST-3" },
  { "Asia/Qatar",           "AST-3" },
  { "Asia/Dhaka",           "BST-6" },
  { "Asia/Kathmandu",       "NPT-5:45" },
  { "Asia/Colombo",         "LKT-5:30" },
  { "Asia/Chennai",         "IST-5:30" },
  { "Asia/Tehran",          "IRST-3:30,IRDT-4:30,M3.0.0,M9.0.0" },
  { "Asia/Sri_Jayawardenepura", "LKT-5:30" },
  { "Asia/Jerusalem",       "IST-2" },
  { "Asia/Tashkent",        "UZT-5" },
  { "Asia/Ho_Chi_Minh",     "ICT-7" },

  { "Africa/Johannesburg",  "SAST-2" },
  { "Africa/Lagos",         "WAT-1" },
  { "Africa/Nairobi",       "EAT-3" },
  { "Africa/Cairo",         "EET-2" },
  { "Africa/Khartoum",      "CAT-2" },
  { "Africa/Abidjan",       "GMT0" },
  { "Africa/Accra",         "GMT0" },
  { "Africa/Dakar",         "GMT0" },
  { "Africa/Lomé",          "GMT0" },
  { "Africa/Monrovia",      "GMT0" },
  { "Africa/Casablanca",    "WET0WEST,M },.2.0,M10.5.0" },

  { "Australia/Sydney",     "AEDT-11,AEST-10,M10.1.0,M4.1.0" },
  { "Australia/Melbourne",  "AEDT-11,AEST-10,M10.1.0,M4.1.0" },
  { "Australia/Brisbane",   "AEST-10" },
  { "Australia/Hobart",     "AEDT-11,AEST-10,M10.1.0,M4.1.0" },
  { "Australia/Perth",      "AWST-8" },

  { "Pacific/Auckland",     "NZDT-13,NZST-12,M9.5.0,M4.1.0" },
  { "Pacific/Fiji",         "FJT-12,FJST-13,M11.1.0,M1.1.0" },
  { "Pacific/Honolulu",     "HST-10" },
  { "Pacific/Guam",         "ChST-10" },
  { "Pacific/Port_Moresby", "PGT-10" },
  { "Pacific/Chatham",      "CHAST-12:45,CHAT-13,M9.5.0,M4.1.0" },
  { "Pacific/Tarawa",       "GILT-10" },
  { "Pacific/Kwajalein",    "MHT-12" },
  { "Pacific/Efate",        "VUT-11" },
  { "Pacific/Truk",         "CHST-10" },
  { "Pacific/Palau",        "PWT-9" },
  { "Pacific/Marquesas",    "MART-9:30" },
  { "Pacific/Pago_Pago",    "SST-11" },
  { "Pacific/Wake",         "WAKT-12" },
  { "Pacific/Johnston",     "HST-10" },
  { "Pacific/Funafuti",     "TVT-12" },

  { "Autodetect", "UTC0" }, // end with default zone
};

uint16_t nOfTimezones = 0;
uint16_t getTimezonesNum() { return nOfTimezones; }

void timezoneInit() {
  while(timeZone[nOfTimezones].ianaTZ != "Autodetect") nOfTimezones++;
}

String getIanaTZ(int tz_index) {
  if (tz_index < TIMEZONE_NUM) {
    return timeZone[tz_index].ianaTZ;
  }

  return "";
}

String getPosixTZ(String ianaTZ) {
  int i = 0;

  while (timeZone[i].ianaTZ != "Autodetect") {
    if (timeZone[i].ianaTZ == ianaTZ) {
      break;
    }
    i++;
  }
  return timeZone[i].posixTZ;
}
  