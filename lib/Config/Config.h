#pragma once
#include <Arduino.h>
#include <IPAddress.h>

// ── Network ──────────────────────────────────────────────
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern IPAddress LOCAL_IP;
extern IPAddress GATEWAY_IP;
extern IPAddress SUBNET_MASK;

// ── Sensor pins ──────────────────────────────────────────
#define DHT_PIN     D6
#define DHT_TYPE    DHT11

#define MQ5_DO_PIN  D7
#define MQ5_AO_PIN  A0

#define PIR_PIN     D5

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
