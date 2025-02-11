#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include "LittleFS.h"
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

ESP8266WebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";

const char* mdns_name = "espclock2";

//Variables to save values from HTML form
String ssid;
String pass;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 255, 0);

unsigned long ota_progress_millis = 0;
// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = 2;
// Stores LED state

String ledState;

boolean restart = false;
bool mdns_on = false;
bool accessPoint = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
  }
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}

// Initialize WiFi
bool initWiFi() 
{
  if(ssid=="")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.print("Connecting to WiFi...");
  for(int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++){
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect.");
    return false;
  }

  Serial.print("LocalIP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

void handleRoot(void)
{    
  Serial.print("Root uri: ");
  Serial.println(server.uri());
  // Just serve the index page from SPIFFS when asked for
  File file = LittleFS.open("/index.html", "r");

  if(!digitalRead(ledPin)) {
    ledState = "ON";
  }
  else {
    ledState = "OFF";
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    int index = fileContent.indexOf("%STATE%");
    if(index >= 0)
    {
      fileContent.replace("%STATE%", ledState);
      Serial.println("State parameter updated");
    }
    server.sendContent(fileContent);
    //break;
  }

  //server.streamFile(file, "text/html");
  file.close();
}

void handleLedOn(void)
{
  Serial.println("Turn Led ON");
  digitalWrite(ledPin, LOW);
  handleRoot();
}

void handleLedOff(void)
{
  Serial.println("Turn Led OFF");
  digitalWrite(ledPin, HIGH);
  handleRoot();
}

void handleCss(void) 
{
  Serial.print("CSS url: ");
  Serial.println(server.uri());
  File file = LittleFS.open("/style.css", "r");
  server.streamFile(file, "text/css");
  file.close();
}

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
      File file = LittleFS.open("/wifimanager.html", "r");
      
      if(!file)
        Serial.println("file reading error");
      else
        Serial.println("File opened for reading");
      server.streamFile(file, "text/html");
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

      if (paramName == PARAM_INPUT_1) {
        ssid = paramValue;
        Serial.print("SSID set to: ");
        Serial.println(ssid);
        // Write file to save value
        writeFile(LittleFS, ssidPath, ssid.c_str());
      }
      if (paramName == PARAM_INPUT_2) {
        pass = paramValue;
        Serial.print("Password set to: ");
        Serial.println(pass);
        // Write file to save value
        writeFile(LittleFS, passPath, pass.c_str());
      }
    }
    if (ssid != "" && pass != "")
    {
      restart = true;
      server.send(200, "text/plain", "Done. ESP will restart, connect to your router and go to: " + (String)mdns_name + ".local");
    }
    else
    {

    }
  }
}


void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  initFS();

  // Set GPIO 2 as an OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
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
    server.on("/on", handleLedOn);
    server.on("/off", handleLedOff);
    server.on("/style.css", handleCss);
  }
  else 
  {
    accessPoint = true;
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESPCLOCK2-AP", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", handleWiFiManager);
    server.on("/style.css", handleCss);
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

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  ElegantOTA.loop();
  
  if (accessPoint == false)
  {
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
  }
  //ElegantOTA.loop();
  if(mdns_on) MDNS.update();
  if (restart){
    delay(5000);
    ESP.restart();
  }
}
