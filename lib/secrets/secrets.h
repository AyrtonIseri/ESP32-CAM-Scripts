#include <pgmspace.h>

#define SECRET
#define CLIENT ""
#define STORE ""
#define CLUSTER ""
#define CAM_ID ""
#define THINGNAME "camera-" CLIENT "-" STORE "-" CLUSTER "-" CAM_ID
#define AWS_IOT_PUBLISH_TOPIC "images/" THINGNAME "/payload/url/get"
#define AWS_IOT_SUBSCRIBE_TOPIC "images/" THINGNAME "/payload/url/accepted"
#define SLEEP_TIME 10000 //milliseconds

#define REBOOTSIZE 3
double REBOOT_TIMES[REBOOTSIZE] = {7, 12, 17}; //times of the day where the camera will reboot

double WORKING_HOURS[2] = {6.5, 22}; //Working hours of the camera

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