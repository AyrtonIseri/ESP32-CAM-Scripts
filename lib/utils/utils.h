#include <Arduino.h>
#include "esp_camera.h"

#ifndef UTIL_H
#define UTIL_H

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