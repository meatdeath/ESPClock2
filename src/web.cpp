#include "main.h"


//Variables to save values from HTML form
String ssid = "";
String pass = "";
String language = "en";

const char* mdns_name = MDNS_NAME;

// Timer variables
const long interval = WAIT_WIFI_TIME;  // interval to wait for Wi-Fi connection (milliseconds)

int8_t networkNum = 0;
networkInfo_t networkInfo[MAX_NETWORKS];

bool accessPoint = false;
bool mdns_on = false;

#ifdef ESP8266
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// ****************************************************************************

// Initialize WiFi
bool initWiFi() 
{
    if(ssid=="")
    {
        Serial.println("SSID undefined.");
        return false;
    }

    Serial.printf("Connecting to WiFi \"%s\"...", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    for(int i = 0; i < WAIT_WIFI_TIME && WiFi.status() != WL_CONNECTED; i++)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Failed to connect.");
        return false;
    }

    Serial.print("Connected. Local IP: ");
    Serial.println(WiFi.localIP());
    return true;
}

// ----------------------------------------------------------------------------

void handleReset(void)
{
    prefs.putString("ssid", "");
    prefs.putString("pass", "");
    {
        restart = true;
        server.send(200, "text/plain", "Done. ESP will restart, connect to AP and use 192.168.4.1 to manage WiFi connections");
    }
}

// ----------------------------------------------------------------------------

void handleRestart(void)
{
    restart = true;
    server.send(200, "text/plain", "Done. ESP will restart now.");
}

// ----------------------------------------------------------------------------

void handleRoot(void)
{    
    Serial.print("Root uri: ");
    Serial.println(server.uri());
    HTTPMethod method = server.method();
    Serial.print("Method: ");
    if(method == HTTP_GET)
    {
        Serial.println("HTTP_GET");
    }
    else if(method == HTTP_POST)
    {
        Serial.println("HTTP_POST");
        int params = server.args();
        Serial.print("N of params: ");
        Serial.println(params);
        for (int i = 0; i < params; i++)
        {
            Serial.print("Param: ");
            String paramName = server.argName(i);
            Serial.print(paramName);
            Serial.print("=");
            String paramValue = server.arg(i);
            Serial.println(paramValue);
            if (paramName == "language")
            {
                if (paramValue == "ru" || paramValue =="en")
                {
                    language = paramValue;
                    prefs.putString("language", language);
                }
                else
                {
                    Serial.println("Unknown language");
                }
            }
            else
            {
                Serial.println("Unknown parameter");
            }
        }
        server.send(200, "text/plain", language);
        return;
    }

    // Just serve the index page from SPIFFS when asked for
    File file;
    if (language == "ru")
    {
        file = LittleFS.open("/index_ru.html", "r");
    }
    else
    {
        file = LittleFS.open("/index.html", "r");
    }

    if(!digitalRead(BLUE_LED_PIN)) {
        if (language == "ru")
            ledState = "ВКЛ";
        else
            ledState = "ON";
    }
    else {
        if (language == "ru")
            ledState = "ВЫКЛ";
        else
            ledState = "OFF";
    }

    //server.sendHeader("Content-type", "text/html");
    server.sendHeader("Content-type", "text/html; charset=utf-8");
    String fileContent;
    while(file.available())
    {
        fileContent = file.readStringUntil('\n');
        String fw_version_template = "%FW_VERSION%";
        String state_template = "%STATE%";
        String min_intencity_template = "%MIN_INTENCITY%";
        String max_intencity_template = "%MAX_INTENCITY%";
        String light_on_min_intencity_template = "%LIGHT_ON_MIN_INTENCITY%";
        String light_on_max_intencity_template = "%LIGHT_ON_MAX_INTENCITY%";

        if (fileContent.indexOf(fw_version_template) >= 0)
        { 
            String version_string = "v." + (String)VERSION_MAJOR + "." + (String)VERSION_MINOR;
            fileContent.replace(fw_version_template, version_string);
        }
        
        if(fileContent.indexOf(state_template) >= 0)
        {
            fileContent.replace(state_template, ledState);
            Serial.println("State parameter updated");
        }

        
// extern uint16_t higher_light;
// extern uint16_t lower_light;
// extern uint8_t lower_intencity;
// extern uint8_t high_intencity;
        if(fileContent.indexOf(min_intencity_template) >= 0)
        {
            fileContent.replace(min_intencity_template, String(lower_intencity));
            Serial.println("State parameter updated");
        }
        if(fileContent.indexOf(max_intencity_template) >= 0)
        {
            fileContent.replace(max_intencity_template, String(high_intencity));
            Serial.println("State parameter updated");
        }
        if(fileContent.indexOf(light_on_min_intencity_template) >= 0)
        {
            fileContent.replace(light_on_min_intencity_template, String(LIGHT_MAX-lower_light));
            Serial.println("State parameter updated");
        }
        if(fileContent.indexOf(light_on_max_intencity_template) >= 0)
        {
            fileContent.replace(light_on_max_intencity_template, String(LIGHT_MAX-higher_light));
            Serial.println("State parameter updated");
        }

        server.sendContent(fileContent);
        //break;
    }

    //server.streamFile(file, "text/html");
    file.close();
}

// ----------------------------------------------------------------------------

void handleLedOn(void)
{
    Serial.println("Turn Led ON");
    digitalWrite(BLUE_LED_PIN, LOW);
    handleRoot();
}

// ----------------------------------------------------------------------------

void handleLedOff(void)
{
    Serial.println("Turn Led OFF");
    digitalWrite(BLUE_LED_PIN, HIGH);
    handleRoot();
}

// ----------------------------------------------------------------------------

void handleTime(void)
{
    server.send(200, "text/plane", timeRead);
}

// ----------------------------------------------------------------------------

void handleTimeOffset(void)
{
    HTTPMethod method = server.method();
    Serial.print("Method: ");
    if(method == HTTP_GET)
    {
        Serial.println("HTTP_GET");
    }
    else if(method == HTTP_POST)
    {
        Serial.println("HTTP_POST");
        int params = server.args();
        Serial.print("N of params: ");
        Serial.println(params);
        if (params == 1)
        {
            Serial.print("Param: ");
            String paramName = server.argName(0);
            Serial.print(paramName);
            Serial.print("=");
            String paramValue = server.arg(0);
            Serial.println(paramValue);

            if (paramName == "seconds")
            {
                timeOffset = paramValue.toInt();
                Serial.print("New time offset: ");
                Serial.println(timeOffset);
                prefs.putInt("timeOffset", timeOffset);
            }
        }
    }
    
    server.send(200, "text/plane", String(timeOffset));
}

// ----------------------------------------------------------------------------

void handleBrightness()
{
    HTTPMethod method = server.method();
    String jsonString = "";
    Serial.print("Method: ");
    if(method == HTTP_GET)
    {
        Serial.println("HTTP_GET");
    }
    else if(method == HTTP_POST)
    {
        Serial.println("HTTP_POST");
        int params = server.args();
        Serial.print("N of params: ");
        Serial.println(params);
        if (params == 4)
        {
            for (int param = 0; param < params; param++)
            {
                Serial.print("Param: ");
                String paramName = server.argName(param);
                Serial.print(paramName);
                Serial.print("=");
                String paramValue = server.arg(param);
                Serial.println(paramValue);

                if (paramName == "min_intencity")
                {
                    lower_intencity = paramValue.toInt();
                    Serial.print("lower_intencity: ");
                    Serial.println(lower_intencity);
                    prefs.putInt("lower_intencity", lower_intencity);
                } 
                else if (paramName == "max_intencity")
                {
                    high_intencity = paramValue.toInt();
                    Serial.print("high_intencity: ");
                    Serial.println(high_intencity);
                    prefs.putInt("high_intencity", high_intencity);
                } 
                else if (paramName == "light_on_min_intencity")
                {
                    lower_light = LIGHT_MAX - paramValue.toInt();
                    Serial.print("lower_light: ");
                    Serial.println(lower_light);
                    prefs.putInt("lower_light", lower_light);
                } 
                else if (paramName == "light_on_max_intencity")
                {
                    higher_light = LIGHT_MAX - paramValue.toInt();
                    Serial.print("higher_light: ");
                    Serial.println(higher_light);
                    prefs.putInt("higher_light", higher_light);
                } 
            }
            jsonString = "{  \"min_intencity\":" + (String)lower_intencity +
                            "\"max_intencity\":" + (String)high_intencity +
                            "\"light_on_min_intencity\":" + (String)(LIGHT_MAX-higher_light) +
                            "\"light_on_max_intencity\":" + (String)(LIGHT_MAX-lower_light)+ "}";
        }
    }    
    
    server.send(200, "text/plane", String(jsonString));
}

// ----------------------------------------------------------------------------

void handleCss(void) 
{
    Serial.print("CSS url: ");
    Serial.println(server.uri());
    File file = LittleFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
}

// ----------------------------------------------------------------------------

void handleWifiManagerJs(void)
{
    Serial.print("JS url: ");
    Serial.println(server.uri());
    File file = LittleFS.open("/wifimanager.js", "r");
    server.streamFile(file, "text/javascript");
    file.close();
}

// ----------------------------------------------------------------------------

void handleJs(void)
{
    Serial.print("JS url: ");
    Serial.println(server.uri());
    File file = LittleFS.open("/index.js", "r");
    server.streamFile(file, "text/javascript");
    file.close(); 
}

// ----------------------------------------------------------------------------

void handleWiFiManager(void)
{
    Serial.print("url: ");
    Serial.println(server.uri());

    HTTPMethod method = server.method();
    Serial.print("Method: ");
    if(method == HTTP_GET)
    {
      Serial.println("HTTP_GET");
        // Just serve the index page from SPIFFS when asked for
        File file;
        if (language == "ru")
        {
            file = LittleFS.open("/wifimanager_ru.html", "r");
        }
        else
        {
            file = LittleFS.open("/wifimanager.html", "r");  
        }

        String fileContent;
        while(file.available())
        {
          fileContent = file.readStringUntil('\n');
          String search_template = "%SSID_LIST%";
          int pos = fileContent.indexOf(search_template);
          if (pos >= 0)
          {
            Serial.printf("%s found at %d\n", search_template.c_str(), pos);
            String substring = fileContent.substring(0, pos);
            server.sendContent(substring);
            Serial.println("begin:\"" + substring + "\"");
            server.sendContent("\n");

            Serial.println("Network num: "+String(networkNum));
            for (int8_t i = 0; i < networkNum; i++)
            {
              String option_string = 
                  String("<option value='") + 
                  networkInfo[i].ssid + 
                  String("'>") + 
                  networkInfo[i].description + 
                  String("</option>\n");
              server.sendContent(option_string);
              Serial.println("Option string: " + String(option_string));
            }
            
            substring = fileContent.substring(pos+search_template.length());
            server.sendContent(substring);
            
            Serial.println("begin:\"" + substring + "\"");
          }
          else
          {
            server.sendContent(fileContent);
          }
        }
        file.close();
    }
    else if(method == HTTP_POST)
    {
        Serial.println("HTTP_POST");
        int params = server.args();
        Serial.print("N of params: ");
        Serial.println(params);

        for(int i = 0; i < params; i++)
        {
            Serial.print("Param: ");
            String paramName = server.argName(i);
            Serial.print(paramName);
            Serial.print("=");
            String paramValue = server.arg(i);
            Serial.println(paramValue);

            if (paramName == "ssid")
            {
                ssid = paramValue;
                Serial.print("SSID set to: ");
                Serial.println(ssid);
            }
            else if (paramName == "pass")
            {
                pass = paramValue;
                Serial.print("Password set to: ");
                Serial.println(pass);
            }
            else if (paramName == "language")
            {
                if (paramValue == "ru" || paramValue =="en")
                {
                    language = paramValue;
                    prefs.putString("language", language);
                    server.send(200, "text/plain", language);
                    return;
                }
                else
                {
                    Serial.println("Unknown language");
                }
            }
            else
            {
                Serial.println("Unknown parameter");
            }
        }
        if (ssid != "" && pass != "")
        {
            prefs.putString("ssid", ssid);
            prefs.putString("pass", pass);
            restart = true;
            server.send(200, "text/plain", "Done. ESP will restart, connect to your router and go to: " + (String)mdns_name + ".local");
            // server.send(200, "text/plain", "Done. ESP will restart, connect to your router and go to: <a href='" + (String)mdns_name + ".local'>" + (String)mdns_name + ".local<\a>");
        }
        else
        {
            server.send(200, "text/plain", "Error. SSID or PASSWORD not selected");
        }
    }
}

void getNetworks(void)
{
    Serial.println(F("Starting WiFi scan..."));

    networkNum = WiFi.scanNetworks(/*async=*/false, /*hidden=*/false);

    if (networkNum == 0) 
    {
        Serial.println(F("No networks found"));
    } 
    else if (networkNum > 0) 
    {
        Serial.printf(PSTR("%d networks found:\n"), networkNum);

        // Print unsorted scan results
        for (int8_t i = 0; i < networkNum && i < MAX_NETWORKS; i++) 
        {
            int32_t rssi;
            uint8_t encryptionType;
            uint8_t *bssid;
            int32_t channel;
            bool hidden;

            #ifdef ESP8266
                WiFi.getNetworkInfo(i, networkInfo[i].ssid, encryptionType, rssi, bssid, channel, hidden);
                networkInfo[i].rssi = rssi;

                // get extra info
                const bss_info *bssInfo = WiFi.getScanInfoByIndex(i);
                String phyMode;
                const char *wps = "";
                if (bssInfo) {
                    phyMode.reserve(12);
                    phyMode = F("802.11");
                    String slash;
                    if (bssInfo->phy_11b) {
                        phyMode += 'b';
                        slash = '/';
                    }
                    if (bssInfo->phy_11g) {
                        phyMode += slash + 'g';
                        slash = '/';
                    }
                    if (bssInfo->phy_11n) {
                        phyMode += slash + 'n';
                    }
                    if (bssInfo->wps) {
                        wps = PSTR("WPS");
                    }
                }
                networkInfo[i].description = 
                    networkInfo[i].ssid + ((encryptionType == ENC_TYPE_NONE)? " " : " (encrypted) ") +
                    " [CH " + String(channel) + "] [" + 
                    String(bssid[0],16) + ":" +  String(bssid[1],16) + ":" + String(bssid[2],16) + ":" + String(bssid[3],16) + ":" + String(bssid[4],16) + ":" + String(bssid[5],16) + "] " + 
                    String(rssi) + "dbm" + (hidden ? " H " : " V ") + phyMode + " " + wps;
            #else // ESP32
                rssi = WiFi.RSSI(i);
                encryptionType = WiFi.encryptionType(i);
                channel = WiFi.channel(i);
                bssid = WiFi.BSSID(i);

                networkInfo[i].ssid = WiFi.SSID(i);
                networkInfo[i].rssi = rssi;
                networkInfo[i].description = 
                    networkInfo[i].ssid + ((encryptionType == WIFI_AUTH_OPEN)? " " : " (encrypted) ") +
                    " [CH " + String(channel) + "] [" + 
                    String(bssid[0],16) + ":" + String(bssid[1],16) + ":" + 
                    String(bssid[2],16) + ":" + String(bssid[3],16) + ":" + 
                    String(bssid[4],16) + ":" + String(bssid[5],16) + "] " + 
                    String(rssi) + "dbm";
            #endif
            //Serial.println(networkInfo[i].description);
            if (i > 0)
            {
                int8_t j;
                for (j = i; j > 0; j--)
                {
                    if (networkInfo[j].rssi <= networkInfo[j-1].rssi)
                    {
                        //Serial.printf("[%d]%d <= [%d]%d\n", j, networkInfo[i].rssi, j-1, networkInfo[j-1].rssi);
                        break;
                    }
                    else
                    {
                        //Serial.printf("swap [%d]%d > [%d]%d\n", j, networkInfo[i].rssi, j-1, networkInfo[j-1].rssi);
                        networkInfo_t tmp = networkInfo[j];
                        networkInfo[j] = networkInfo[j-1];
                        networkInfo[j-1] = tmp;
                    }
                }
            }
            yield();
        }
        
        Serial.println("Sorted list:");
        for (int8_t i = 0; i < networkNum && i < MAX_NETWORKS; i++) 
        {
            Serial.println(networkInfo[i].description);
        }
    } 
    else 
    {
        Serial.printf(PSTR("WiFi scan error %d"), networkNum);
    }
}

void handleOrientationRequest(void)
{
    HTTPMethod method = server.method();
    Serial.print("Method: ");
    if(method == HTTP_GET)
    {
        Serial.println("HTTP_GET");
    }
    else if(method == HTTP_POST)
    {
        Serial.println("HTTP_POST");
        int params = server.args();
        Serial.print("N of params: ");
        Serial.println(params);
        if (params == 1)
        {
            Serial.print("Param: ");
            String paramName = server.argName(0);
            Serial.print(paramName);
            Serial.print("=");
            String paramValue = server.arg(0);
            Serial.println(paramValue);

            if (paramName == "orientation") {
                matrix_orientation = (matrix_orientation_t)paramValue.toInt();
                if (matrix_orientation < MATRIX_ORIENTATION_0 || matrix_orientation > MATRIX_ORIENTATION_CCW90)
                {
                    matrix_orientation = MATRIX_ORIENTATION_0;
                }
                Serial.print("New orientation: ");
                Serial.println(matrix_orientation);
                prefs.putInt("orientation", matrix_orientation);
            }
        }
    }
  
    server.send(200, "text/plane", String(matrix_orientation));
}

