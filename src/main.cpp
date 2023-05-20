#include <Arduino.h>
#include <WiFiMulti.h>

#define WIFI_SSID "network"
#define WIFI_PASSWORD "password"

 WiFiMulti wifimulti;

void setup() {
  Serial.begin(115200);

  wifimulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
  while (wifimulti.run() != WL_CONNECTED) {
    Serial.println("Connection couldn't be established. Retrying... ");
    delay(1000);
  }

  Serial.println("Connected!");
}

void loop() {
  delay(2000);

  if (WiFi.status() == WL_CONNECTED)
    Serial.println("The device is communicating with the internet");
  else
    Serial.println("Lost communication. Reconnecting...");

}
