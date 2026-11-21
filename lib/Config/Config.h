#pragma once
#include <Arduino.h>
#include <IPAddress.h>

// ── Network ──────────────────────────────────────────────
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern IPAddress LOCAL_IP;
extern IPAddress GATEWAY_IP;
extern IPAddress SUBNET_MASK;

extern const char* MQTT_HOST;
extern uint16_t MQTT_PORT;
extern const char* MQTT_USER;
extern const char* MQTT_PASS;
extern const char* MQTT_DEVICE_ID;
extern const char* MQTT_PREFIX;


// ── Sensor pins ──────────────────────────────────────────
#define DHT_TYPE    DHT11

#define PIR_PIN     D8       // GPIO15
#define DHT_PIN     3        // GPIO3 / RX; avoids D0 and all boot-strap pins
#define MQ5_AO_PIN  A0       // Analog output; no other sensor may use A0

#define FAN_IR_PIN  1        // GPIO1 / TX; serial logging is disabled

// Hardware SPI uses D5 (SCK), D6 (MISO), and D7 (MOSI).
// D3 / GPIO0 is the SD chip-select pin and needs an external 10k pull-up.
#define SD_CS_PIN   D3


// ── Relay pins ───────────────────────────────────────────
#define STAIR_LAMP_RELAY_PIN   D1
#define LIVING_ROOM_RELAY_PIN  D2

// Most opto-isolated relay modules are active-low: LOW energizes the relay.
#define RELAY_ON  LOW
#define RELAY_OFF HIGH


// ── Timing constants ─────────────────────────────────────
#define GAS_WARMUP_MS        10000UL
#define GAS_OFFLINE_THRESHOLD 15
#define GAS_ALERT_THRESHOLD  400
#define STAIR_AUTO_OFF_MS    20000UL
#define CLIMATE_READ_MS      5000UL
#define GAS_READ_MS          1000UL
