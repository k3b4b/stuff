#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Servo.h>

const char* ssid = "k3b4b";
const char* password = "90229022";
char* macraw = "b0:6e:bf:35:e3:70";

AsyncWebServer server(3035);
WiFiUDP udp;

void setup() {
  if(!SPIFFS.begin()){
    Serial.println("An error has occured while mountinh SPIFFS");
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
  });
  ArduinoOTA.begin();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/startpc", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
    sendWOL();
  });
  
  server.begin();
}

void loop() {
  ArduinoOTA.handle();

}

//wake on lan по маку
void sendWOL() {
  byte mac[6];
  char* ptr;
 
  mac[0] = strtol( strtok(macraw,":"), &ptr, HEX );
  for( uint8_t i = 1; i < 6; i++ )
  {
    mac[i] = strtol( strtok( NULL,":"), &ptr, HEX );
  }
  
  const int magicPacketLength = 102;
  byte magicPacket[magicPacketLength] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  for (uint8_t ix=6; ix < magicPacketLength; ix++)
    magicPacket[ix] = mac[ix % 6];
  
  udp.begin(12345);
  udp.beginPacket("192.168.1.255", 9);
  udp.write(magicPacket, magicPacketLength);
  udp.endPacket();
  udp.stop();
}
