#include <Arduino.h>
#include <ESP_WiFiManager.h>
#include <WiFiClientSecure.h>
#include <secrets.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>

String AP_SSID = "AutoConnectAP";
String AP_PASS = "password";

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void connectWifi() {
  ESP_WiFiManager ESP_wifiManager;
  if (!ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str()))
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  else
    Serial.println(F("WiFi connected...yeey :)"));
}

void publishURLRequest() {
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["client"] = CLIENT;
  doc["key"] = "photo";

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.print("publishing on ");
  Serial.print(AWS_IOT_PUBLISH_TOPIC);
  Serial.print(" message: ");
  Serial.println(jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

   StaticJsonDocument<200> doc;
   deserializeJson(doc, payload);
   const char* message = doc["message"];
   Serial.println(message);
}

void connectAWS() {  
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

    if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void setup() {
  Serial.begin(115200);
  connectWifi();
  connectAWS();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("The device is communicating with the internet");
    publishURLRequest();
    client.loop();
  }

  else {
    Serial.println("Lost communication. Reconnecting...");
  }

  delay(10000);
}
