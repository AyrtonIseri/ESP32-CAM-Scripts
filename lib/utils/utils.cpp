#include <Arduino.h>
#include <secrets.h>
#include <utils.h>

void printHeap() {
  Serial.print("Current Heap free space: ");
  Serial.println(ESP.getFreeHeap());
}

void deep_sleep(long time_to_sleep) {
  esp_sleep_enable_timer_wakeup(time_to_sleep);

  Serial.println("Setup ESP32 to sleep for " + String(time_to_sleep / MICRO_TO_SECONDS) + " Seconds");
  Serial.flush();

  esp_deep_sleep_start();
}

void espRestart() {
  delay(1000);
  Serial.flush();
  ESP.restart();
  // deep_sleep(1000);
}
