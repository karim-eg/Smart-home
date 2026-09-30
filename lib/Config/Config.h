#pragma once
#include <Arduino.h>
#include <IPAddress.h>

// ── Network ──────────────────────────────────────────────
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern IPAddress LOCAL_IP;
extern IPAddress GATEWAY_IP;
extern IPAddress SUBNET_MASK;

// MQTT connection settings, populated from the ignored secrets.h file.
extern const char* MQTT_HOST;
extern uint16_t MQTT_PORT;
extern const char* MQTT_USER;
extern const char* MQTT_PASS;
extern const char* MQTT_DEVICE_ID;
extern const char* MQTT_PREFIX;


// ── Sensor pins ──────────────────────────────────────────
#define DHT_TYPE    DHT11

#define PIR_PIN     D5
#define DHT_PIN     D6
#define MQ5_DO_PIN  D7

#define SOIL_PIN    A0
#define PUMP_PIN    D8


// ── Relay pins ───────────────────────────────────────────
#define STAIR_LAMP_RELAY_PIN   D1
#define LIVING_ROOM_RELAY_PIN  D2

#define RELAY_ON  HIGH
#define RELAY_OFF LOW


// ── Timing constants ─────────────────────────────────────
#define GAS_WARMUP_MS        10000UL
#define GAS_OFFLINE_THRESHOLD 15
#define STAIR_AUTO_OFF_MS    20000UL
#define CLIMATE_READ_MS      5000UL
#define GAS_READ_MS          1000UL
#define SOIL_DRY_THRESHOLD   697
#define PUMP_TIME_MS         500UL
#define SOIL_SOAK_MS         6000UL
#define SOIL_READ_MS         2000UL
