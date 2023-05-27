#include <secrets.h>
#include <timing.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP_WiFiManager.h>
#include <WiFiClientSecure.h>
#include <camera.h>

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(MQTT_BUF_SIZE);

bool RECEIVED_POST_URL = false;

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
  int tries = 1;

  while (result < 0){
    delay(1000);
    result = http.PUT(fb->buf, fb->len);
    tries += 1;
    if (tries > 5)
      ESP.restart();
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

  uploadFileToS3(url["url"], fb);
  
  esp_camera_fb_return(fb);
  delay(100);
  esp_camera_fb_return(fb);
}

void connectAWS() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.setKeepAlive(MQTT_KEEP_ALIVE_TIME);
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

  if (WiFi.status() != WL_CONNECTED)
    connectWifi();

  while(!client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer)) {
    Serial.println("Couldn't publish the request, trying again");
    delay(1000);
    client.loop();
    if (!client.connected())
      connectAWS();
  }

  Serial.print("MQTT message successfully sent.");
}
