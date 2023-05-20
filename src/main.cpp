#include <Arduino.h>
#include <WiFiMulti.h>
#include <ESP_WiFiManager.h>

#define WIFI_SSID "network"
#define WIFI_PASSWORD "password"

String AP_SSID = "AutoConnectAP";
String AP_PASS = "password";

void setup() {
  Serial.begin(115200);

  ESP_WiFiManager ESP_wifiManager;
  if (!ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str()))
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  else
    Serial.println(F("WiFi connected...yeey :)"));

}

void loop() {
  delay(2000);

  if (WiFi.status() == WL_CONNECTED)
    Serial.println("The device is communicating with the internet");
  else
    Serial.println("Lost communication. Reconnecting...");

}
