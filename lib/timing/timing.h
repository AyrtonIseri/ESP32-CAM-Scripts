#include <TimeLib.h>
#include <Arduino.h>
#include <secrets.h>
#include <utils.h>

#ifndef TIMING_H
#define TIMING_H

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    espRestart();
  }
  time(&now);
  return now;
}

void syncTime() {
  configTime(GMT_TIMEZONE_OFFSET * 3600, 0, "pool.ntp.org");
  Serial.print("Setting up adequate time");

  while (!time(nullptr)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime successfully setted!\n");
}

unsigned long getCurrentHour() {
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    espRestart();
  }

  char timeHour[3];
  char timeMinute[3];
  char timeSeconds[3];
  
  strftime(timeHour,3, "%H", &timeinfo);
  strftime(timeMinute,3, "%M", &timeinfo);
  strftime(timeSeconds,3, "%S", &timeinfo);

  unsigned long currentHour = atoi(timeHour) * 3600 + atoi(timeMinute) * 60 + atoi(timeSeconds);

  return currentHour;
}

void waitAndReboot(long timeToMonitor) {
  Serial.println("Soon, the device will restart! \n");
  unsigned long currentHour = getCurrentHour();

  while (timeToMonitor - currentHour > REBOOT_SAFETY_MARGIN)
    currentHour = getCurrentHour();

  espRestart();
}

void verifyReboot() {
  unsigned long currentHour = getCurrentHour();

  int REBOOTSIZE = sizeof(REBOOT_TIMES) / sizeof(REBOOT_TIMES[0]);

  for (int i = 0; i < REBOOTSIZE; i++) {
    long time = REBOOT_TIMES[i] * 3600;

    if (currentHour <= time)
      if (time - currentHour <= SLEEP_TIME + REBOOT_TIME_MARGIN)
        waitAndReboot(time);

  }

}

bool isSleepyHour(unsigned long currentHour) {
  return (currentHour < WORKING_HOURS[0] * 3600 || currentHour > WORKING_HOURS[1] * 3600);
}

bool isCloseToWakingUp(unsigned long currentHour) {
  return (currentHour < WORKING_HOURS[1]*3600 && 
  currentHour < WORKING_HOURS[0] && 
  WORKING_HOURS[0] * 3600 - currentHour < WORKING_HOUR_SLEEP);
}

void verifyWorkingHours() {
  unsigned long currentHour = getCurrentHour();
  if (isSleepyHour(currentHour)) {
    
    if (isCloseToWakingUp(currentHour))
      waitAndReboot(WORKING_HOURS[0] * 3600);

    Serial.println("Current Period is not a working hour. Going to sleep");
    deep_sleep(WORKING_HOUR_SLEEP * MICRO_TO_SECONDS);
  }
}

#endif