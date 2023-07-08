#ifndef TIMING_H
#define TIMING_H

unsigned long getTime();

void syncTime();

unsigned long getCurrentHour();

void waitAndReboot(long timeToMonitor);

void verifyReboot();

bool isSleepyHour(unsigned long currentHour);

bool isCloseToWakingUp(unsigned long currentHour);

void verifyWorkingHours();

#endif