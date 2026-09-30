#include "Config.h"
#include "secrets.h"

#ifndef MQTT_BROKER_HOST
#define MQTT_BROKER_HOST ""
#endif
#ifndef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT 1883
#endif
#ifndef MQTT_USERNAME
#define MQTT_USERNAME ""
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD ""
#endif
#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "smart-home-esp8266"
#endif
#ifndef MQTT_TOPIC_PREFIX
#define MQTT_TOPIC_PREFIX "smart-home/home"
#endif

IPAddress LOCAL_IP(192, 168, 100, 50);
IPAddress GATEWAY_IP(192, 168, 100, 1);
IPAddress SUBNET_MASK(255, 255, 255, 0);

const char* MQTT_HOST = MQTT_BROKER_HOST;
uint16_t MQTT_PORT = MQTT_BROKER_PORT;
const char* MQTT_USER = MQTT_USERNAME;
const char* MQTT_PASS = MQTT_PASSWORD;
const char* MQTT_DEVICE_ID = MQTT_CLIENT_ID;
const char* MQTT_PREFIX = MQTT_TOPIC_PREFIX;
