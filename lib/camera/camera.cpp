#include <Arduino.h>
#include <secrets.h>
#include <utils.h>
#include <errors.h>
#include <connection.h>
#include "camera.h"
#include "esp_camera.h"

void configCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 5;
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  int tries = 0;

  while (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x. Retrying... \n", err);
    tries += 1;
    if (tries>10) {
      Serial.printf("Camera init failed with error 0x%x", err);

      String messageError = "Camera device couldn't be initiated. Error code returned was: 0x" + String(err);      
      baseError * cError = cameraConfigError(messageError);
      char * jsonBuffer = getErrorLog(cError);

      String errorTopic = DEFAULT_ERROR_TOPIC + cError->errorType;
      publishMessage(errorTopic, jsonBuffer);
    
      delete cError;
      delete[] jsonBuffer;

      espRestart();
    }
    delay(2000);
    err = esp_camera_init(&config);
  }

  sensor_t * s = esp_camera_sensor_get();

  Serial.print("Setting up camera device");
  for (int i = 0; i < 3; i ++) {
    camera_fb_t * configfb = esp_camera_fb_get();
    delay(1000);
    esp_camera_fb_return(configfb);
    Serial.print(".");
  }

  Serial.println(" Camera successfully configured");
}