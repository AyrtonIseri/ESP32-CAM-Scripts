#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP_WiFiManager.h>
#include <WiFi.h>

#include <secrets.h>
#include <camera.h>
#include <timing.h>
#include <errors.h>
#include <utils.h>
#include <connection.h>

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(MQTT_BUF_SIZE);

bool RECEIVED_POST_URL = false;

void publishMessage(String topic, char * payload);

void connectWifi() {
  unsigned long startTime = millis();
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long currentTime = millis();
    
    if (currentTime - startTime > WIFI_CONNECTION_TIMEOUT * SECONDS_TO_MILLI)
      espRestart();

    Serial.println("Couldn't establish connetion. Retrying...");
    delay(500);
  }

  Serial.println("Wifi Connected");

}

void uploadFileToS3(String uploadUrl, camera_fb_t* fb) {
  HTTPClient http;
  http.begin(uploadUrl);
  http.setConnectTimeout(HTTP_CONNECT_TIMEOUT * SECONDS_TO_MILLI);

  Serial.print("payload size: ");
  Serial.print(fb->len);
  Serial.println(".");

  int result = http.PUT(fb->buf, fb->len);
  int tries = 1;

  while (result < 0){
    delay(1000);

    result = http.PUT(fb->buf, fb->len);
    Serial.print("Current HTTP status: ");
    Serial.println(http.connected());

    if(WiFi.status() != WL_CONNECTED)
      connectWifi();

    tries += 1;
    if (tries > 10){
      Serial.println("Couldn't PUT request the file. Rebooting");

      String messageError = "Camera device couldn't PUT request the file to the cloud. \n Current HTTP connection status: " + String(http.connected());      
      baseError * cError = httpError(messageError);
      char * jsonBuffer = getErrorLog(cError);

      String errorTopic = DEFAULT_ERROR_TOPIC + cError->errorType;
      publishMessage(errorTopic, jsonBuffer);

      delete cError;
      delete[] jsonBuffer;

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

  if (!fb) {
    Serial.println("Camera capture failed! Rebooting");

    String messageError = "Camera capture has failed and the device will restart. Current heap level: " + String(ESP.getFreeHeap());
    baseError * cError = cameraError(messageError);
    char * jsonBuffer = getErrorLog(cError);

    String errorTopic = DEFAULT_ERROR_TOPIC + cError->errorType;
    publishMessage(errorTopic, jsonBuffer);

    delete cError;
    delete[] jsonBuffer;

    esp_camera_fb_return(fb);
    espRestart();
  }

  uploadFileToS3(url["url"], fb);

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

  int tries = 0;
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(200);
    tries += 1;
    if (tries > 100)
      espRestart();
  }
  
  Serial.print("\n");
  
  if(!client.connected()){
    Serial.println("\nAWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
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

  publishMessage(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  Serial.println("MQTT message successfully sent.");
}

void publishMessage(String topic, char * payload) {
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();

  int tries = 0;
  while(!client.publish(topic, payload)) {

    Serial.println("Couldn't publish the request, trying again");
    delay(1000);
    client.loop();
    
    if (!client.connected())
      connectAWS();

    tries += 1;

    if (tries > 30)
      espRestart();
  }
}