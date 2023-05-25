#include <Arduino.h>
#include <ESP_WiFiManager.h>
#include <WiFiClientSecure.h>
#include <secrets.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TimeLib.h>
#include "esp_camera.h"

#define MQTT_BUF_SIZE 7168
#define SLEEP_TIME 60000

#define GMT_TIMEZONE_OFFSET -3

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
MQTTClient client = MQTTClient(MQTT_BUF_SIZE);

bool RECEIVED_POST_URL = false;
unsigned long millisNow = -SLEEP_TIME;

camera_fb_t * fb;

void connectWifi() {
  ESP_WiFiManager ESP_wifiManager;
  if (!ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str()))
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  else
    Serial.println(F("\nWiFi connected...yeey :)\n"));
}

void uploadFileToS3(String uploadUrl, camera_fb_t* fb) {
  HTTPClient http;
  http.begin(uploadUrl);

  Serial.print("payload size: ");
  Serial.print(fb->len);
  Serial.println(".");

  int result = http.PUT(fb->buf, fb->len);
  Serial.print("The result from the post operation was: ");
  Serial.print(result);
  Serial.println(".");
  
  Serial.print("Request response content: \n");
  Serial.println(http.getString());

  http.end();
}

void messageHandler(String &topic, String &payload) {
  RECEIVED_POST_URL = true;

  StaticJsonDocument<2048> url;
  DeserializationError error = deserializeJson(url, payload);

  if (error) {
    Serial.print("Error parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }

  uploadFileToS3(url["url"], fb);
  
  esp_camera_fb_return(fb);
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

  Serial.println("\nAWS IoT Connected!\n");
}

void publishURLRequest(String objName) {
  StaticJsonDocument<200> doc;
  doc["client"] = CLIENT;
  doc["topic"] = AWS_IOT_SUBSCRIBE_TOPIC;
  doc["key"] = objName;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.print("PUT request to upload the following file: ");
  Serial.println(objName);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void configCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 5;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  sensor_t * s = esp_camera_sensor_get();
}

void syncTime() {
  configTime(GMT_TIMEZONE_OFFSET * 3600, 0, "pool.ntp.org");
  Serial.print("Setting up adequate time");

  while (!time(nullptr)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime successfully setted!\n");
}

void setup() {
  Serial.begin(115200);
  connectWifi();
  connectAWS();
  syncTime();
  configCamera();
}

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

String getObjectName() {
  time_t currTimestamp = getTime();
  String strTimestamp = String(currTimestamp);

  String objName = THINGNAME + String("-") + strTimestamp + String(".png");

  return objName;
}

void deep_sleep() {
  delay(SLEEP_TIME);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected())
      connectAWS();

    if (millis() > millisNow + SLEEP_TIME) {
      
      String objName = getObjectName();
      fb = esp_camera_fb_get();

      if (!fb) {
        Serial.println("Camera capture failed");
        delay(1000);
        return;
      }
      
      publishURLRequest(objName);
      
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
  deep_sleep();
}