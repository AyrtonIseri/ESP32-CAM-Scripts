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
  printHeap();
  connectWifi();
  connectAWS();
  syncTime();
  configCamera();
  printHeap();
}

void loop() {
  Serial.println("Entering outter loop");
  long millisnow = millis();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Entering inner loop");
    verifyWorkingHours();
    Serial.println("Verified working hours");

    verifyReboot();
    Serial.println("Reboot necessity verified");

    if (!client.connected())
      connectAWS();
    
    Serial.println("Initiating camera");
    String objName = getObjectName();
    printHeap();

    publishURLRequest(objName);
    
    while (!RECEIVED_POST_URL){
      client.loop();
    }

    RECEIVED_POST_URL = false;

  }

  long timeDelay = millis() - millisnow;

  deep_sleep(SLEEP_TIME * MICRO_TO_SECONDS - timeDelay * MICRO_TO_MILLI);
}