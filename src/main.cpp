#include <Arduino.h>
#include <secrets.h>
#include <connection.h>
#include <camera.h>

String getFileTitle() {
  time_t currTimestamp = getTime();
  String strTimestamp = String(currTimestamp);

  String fileName = THINGNAME + String("-") + strTimestamp + String(".png");
  return fileName;
}

void loopMQTTAndRestartIfTimeout(long timeSinceWaitingBegan) {
    client.loop();

    long elapsedTime = millis() - timeSinceWaitingBegan; //milliseconds
    long waitingTimeout = SLEEP_TIME * SECONDS_TO_MILLI; //milliseconds

    if (elapsedTime > waitingTimeout)
      espRestart();
}

void listenForURL(){
  long timeSinceWaitingBegan = millis();
  while (!RECEIVED_POST_URL)
    loopMQTTAndRestartIfTimeout(timeSinceWaitingBegan);
}

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
    
    String fileTitle = getFileTitle();
    publishURLRequest(fileTitle);
    listenForURL();
  }

  long timeDelayDueToExecution = millis() - millisnow;
  deep_sleep(SLEEP_TIME * MICRO_TO_SECONDS - timeDelayDueToExecution * MICRO_TO_MILLI);
}