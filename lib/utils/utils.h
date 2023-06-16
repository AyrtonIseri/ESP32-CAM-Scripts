#include <Arduino.h>
#include "esp_camera.h"

#ifndef UTIL_H
#define UTIL_H

void deep_sleep(long time_to_sleep) {
  esp_sleep_enable_timer_wakeup(time_to_sleep);

  Serial.println("Setup ESP32 to sleep for " + String(time_to_sleep / MICRO_TO_SECONDS) + " Seconds");
  Serial.flush();

  esp_deep_sleep_start();
}

void printHeap() {
  Serial.print("Current Heap free space: ");
  Serial.println(ESP.getFreeHeap());
}

void espRestart() {
  delay(1000);
  Serial.flush();
  
  ESP.restart();
}

#endif