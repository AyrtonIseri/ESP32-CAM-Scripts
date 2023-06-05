#include <secrets.h>
#include <timing.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP_WiFiManager.h>
#include <WiFiClientSecure.h>

#ifndef CONNECTION_H
#define CONNECTION_H

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(MQTT_BUF_SIZE);

bool RECEIVED_POST_URL = false;

void rebootLog(String messageError);

void connectWifi() {
  ESP_WiFiManager ESP_wifiManager;
  // ESP_wifiManager.resetSettings();
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
  int tries = 1;

  while (result < 0){
    delay(1000);
    result = http.PUT(fb->buf, fb->len);
    tries += 1;
    if (tries > 10){
      rebootLog("Couldn't put request the file after 10 tries. Rebooting!");
      http.end();
      espRestart();
    }
  }

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

  camera_fb_t* fb = esp_camera_fb_get();
  Serial.println("Photo taken and stored");
  printHeap();

  if (!fb) {
    Serial.println("Camera capture failed");
    printHeap();
    rebootLog("Couldn't take picture, rebooting system!");
    esp_camera_fb_return(fb);
    espRestart();
  }

  uploadFileToS3(url["url"], fb);
  
  esp_camera_fb_return(fb);
  printHeap();
}

void sendLogStatus() {
  StaticJsonDocument<200> doc;
  doc["status"] = "Connected";
  doc["device"] = THINGNAME;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  if (WiFi.status() != WL_CONNECTED)
    connectWifi();

  while(!client.publish(AWS_IOT_LOGS_TOPIC, jsonBuffer)) {
    Serial.println("\nCouldn't log the operation, skipping log part");
    client.loop();

    if (!client.connected())
      return;

  }
}

void connectAWS() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.setKeepAlive(MQTT_KEEP_ALIVE_TIME);
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  int tries = 0;
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(200);
    tries += 1;
    if (tries > 100)
      espRestart();
  }

    if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  sendLogStatus();

  // Serial.println("\nAWS IoT Connected!\n");
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

  if (WiFi.status() != WL_CONNECTED)
    connectWifi();

  int tries = 0;
  while(!client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer)) {
    Serial.println("Couldn't publish the request, trying again");
    rebootLog("Couldn't publish the request, trying again");
    delay(1000);
    client.loop();
    if (!client.connected())
      connectAWS();
    tries += 1;
    if (tries > 30)
      espRestart();
  }

  Serial.print("MQTT message successfully sent.");
}

void rebootLog(String messageError) {
  StaticJsonDocument<200> doc;
  doc["status"] = "Rebooting";
  doc["device"] = THINGNAME;
  doc["message"] = messageError;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
  
  int tries = 1;
  while(!client.publish(AWS_IOT_LOGS_TOPIC, jsonBuffer)) {
    client.loop();

    if (!client.connected())
      connectAWS();
    
    tries += 1;
    if (tries > 30)
      espRestart();
  }
}

#endif