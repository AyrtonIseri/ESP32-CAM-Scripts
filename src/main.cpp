#include <Arduino.h>
#include <ESP_WiFiManager.h>
#include <WiFiClientSecure.h>
#include <secrets.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "esp_camera.h"


#define MQTT_BUF_SIZE 7168
#define SLEEP_TIME 10000

String AP_SSID = "AutoConnectAP";
String AP_PASS = "password";


// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiClientSecure net = WiFiClientSecure();
WiFiClientSecure wifiClient;
MQTTClient client = MQTTClient(MQTT_BUF_SIZE);

bool RECEIVED_POST_URL = false;
unsigned long millisNow = -SLEEP_TIME;

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
  doc["topic"] = AWS_IOT_SUBSCRIBE_TOPIC;
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
  RECEIVED_POST_URL = true;
  Serial.println(sizeof(payload));
  Serial.println("incoming: " + topic + " - " + payload);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  // Serial.println(payload);
  //  const char* message = doc["message"];
  //  Serial.println(message);
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

  Serial.println("\nAWS IoT Connected!");
}

void deep_sleep() {
  delay(SLEEP_TIME);
}

void setHTTP() {
  const char* bucketName = "supin-nema";
  const char* objectKey = "photo";
  const char* uploadUrl = "https://supin-nema.s3.amazonaws.com/";
  const char* contentType = "image/jpg";
  const char* accessKeyId = "ASIA2E3752DQNPQIBCUQ";
  const char* securityToken = "IQoJb3JpZ2luX2VjEIH...";
  const char* policy = "eyJleHBpcmF0aW9u...";
  const char* signature = "BGsKhBmK/3/8cHqqw2fyUcCUxZ8=";

  HTTPClient http;
  http.begin(wifiClient, uploadUrl);
  http.addHeader("ContentType", contentType);
  http.addHeader("x-amz-security-token", securityToken);
  http.addHeader("AWSAccessKeyId", accessKeyId);
}

void setup() {
  Serial.begin(115200);
  connectWifi();
  connectAWS();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected())
      connectAWS();

    if (millis() > millisNow + SLEEP_TIME) {
      publishURLRequest();

      while (!RECEIVED_POST_URL)
        client.loop();

      RECEIVED_POST_URL = false;
      millisNow = millis();
    }
  }

  else {
    Serial.println("Lost communication. Reconnecting...");
  }
  client.loop();
  // deep_sleep();
}
