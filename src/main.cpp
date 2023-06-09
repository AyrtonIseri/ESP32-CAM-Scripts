#include <Arduino.h>
#include <secrets.h>
#include <connection.h>

String getObjectName() {
  time_t currTimestamp = getTime();
  String strTimestamp = String(currTimestamp);

  String objName = THINGNAME + String("-") + strTimestamp + String(".png");

  return objName;
}
// Adicionar um outro log para o camera probe failed!!!!!!!!!!!!!!!!!!!!!! Assim como no PUT request e no capture taken

void setup() {
  Serial.begin(115200);
  connectWifi();
  connectAWS();
  syncTime();
  configCamera();
}

void loop() {
  long millisnow = millis();
  if (WiFi.status() == WL_CONNECTED) {
    verifyWorkingHours();

    verifyReboot();

    if (!client.connected())
      connectAWS();
    
    String objName = getObjectName();

    publishURLRequest(objName);
    
    long millis_to_received = millis();
    while (!RECEIVED_POST_URL){
      client.loop();
      if (millis() - millis_to_received > SLEEP_TIME * SECONDS_TO_MILLI)
        espRestart();
    }

    RECEIVED_POST_URL = false;

  }

  long timeDelay = millis() - millisnow;
  deep_sleep(SLEEP_TIME * MICRO_TO_SECONDS - timeDelay * MICRO_TO_MILLI);
}