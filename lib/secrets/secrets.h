#include <pgmspace.h>

#ifndef HEADER_H
#define HEADER_H

// Camera definition config
#define SECRET
#define CLIENT ""
#define STORE ""
#define CLUSTER ""
#define CAM_ID ""
#define THINGNAME "camera-" CLIENT "-" STORE "-" CLUSTER "-" CAM_ID

// Camera pub-sub topics definition
#define AWS_IOT_PUBLISH_TOPIC "images/" THINGNAME "/payload/url/get"
#define AWS_IOT_SUBSCRIBE_TOPIC "images/" THINGNAME "/payload/url/accepted"

// Camera sleep time definition
#define SLEEP_TIME 1800 //seconds

//Sleep time config stats
#define REBOOT_TIME_MARGIN 120 //seconds
#define REBOOT_SAFETY_MARGIN 2 //seconds
#define WORKING_HOUR_SLEEP 1800 //seconds

// MQTT Config stats
#define MQTT_BUF_SIZE 7168
#define MQTT_KEEP_ALIVE_TIME 120 //In seconds

//Timezone config stats
#define GMT_TIMEZONE_OFFSET -3

// AP config stats
String AP_SSID = "AutoConnectAP" + String(THINGNAME);
String AP_PASS = "password";

// Defining rebooting options
int REBOOT_TIMES[] = {7, 12, 17}; //times of the day where the camera will reboot
double WORKING_HOURS[] = {6.5, 22}; //Working hours of the camera

const char AWS_IOT_ENDPOINT[] = "";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";

#define MICRO_TO_SECONDS 1000000
#define MICRO_TO_MILLI 1000

#endif