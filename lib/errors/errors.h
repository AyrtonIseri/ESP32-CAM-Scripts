#include <Arduino.h>

#ifndef ERROR_H
#define ERROR_H

struct baseError {
    String errorType;
    String errorMessage;
};

baseError * cameraError(String errorMessage);

baseError * httpError(String errorMessage);

baseError * cameraConfigError(String errorMessage);

char * getErrorLog(baseError * error);

#endif