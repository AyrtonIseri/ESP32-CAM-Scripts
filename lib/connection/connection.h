#include <ArduinoJson.h>
#include "esp_camera.h"
#include <MQTTClient.h>
#include <WiFi.h>
#include "HTTPClient.h"


#ifndef CONNECTION_H
#define CONNECTION_H

extern MQTTClient client;
extern WiFiClientSecure net;
extern bool RECEIVED_POST_URL;

void publishMessage(String topic, char * payload);

void connectWifi();

void uploadFileToS3(String uploadUrl, camera_fb_t* fb);

void messageHandler(String &topic, String &payload);

void connectAWS();

void publishURLRequest(String objName);

#endif