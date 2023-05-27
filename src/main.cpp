#include <Arduino.h>
#include <secrets.h>
#include <connection.h>

String getObjectName() {
  time_t currTimestamp = getTime();
  String strTimestamp = String(currTimestamp);

  String objName = THINGNAME + String("-") + strTimestamp + String(".png");

  return objName;
}

void setup() {
  Serial.begin(115200);
  connectWifi();
  connectAWS();
  syncTime();
  configCamera(fb);
}

void loop() {
  long millisnow = millis();
  if (WiFi.status() == WL_CONNECTED) {
    verifyWorkingHours();

    verifyReboot();

    if (!client.connected())
      connectAWS();

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

  }

  else {
    Serial.println("Lost communication. Reconnecting...");
  }

  long timeDelay = millis() - millisnow;
  deep_sleep(SLEEP_TIME * MICRO_TO_SECONDS - timeDelay * MICRO_TO_MILLI);
}