#include <Arduino.h>
#include <secrets.h>
#include <ArduinoJson.h>

#ifndef ERROR_H
#define ERROR_H

struct baseError {
    String errorType;
    String errorMessage;
};

baseError * cameraError(String errorMessage){
    baseError* new_error = new baseError;
    new_error->errorType = "CAMERA";
    new_error->errorMessage = errorMessage;
    return new_error;
}

baseError * httpError(String errorMessage){
    baseError* new_error = new baseError;
    new_error->errorType = "HTTP";
    new_error->errorMessage = errorMessage;
    return new_error;
}

baseError * cameraConfigError(String errorMessage){
    baseError* new_error = new baseError;
    new_error->errorType = "CAMERA CONFIG";
    new_error->errorMessage = errorMessage;
    return new_error;
}

char * getErrorLog(baseError * error) {
    StaticJsonDocument<600> doc;
    doc["device"] = THINGNAME;
    doc["error type"] = error->errorType;
    doc["message"] = error->errorMessage;


    size_t bufferSize = measureJson(doc) + 1;
    char* jsonBuffer = new char[bufferSize];
    serializeJson(doc, jsonBuffer, bufferSize);

    return jsonBuffer;
}



#endif