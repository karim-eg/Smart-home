#include <Arduino.h>
#include "Config.h"
#include "ClimateSensor.h"
#include "GasSensor.h"
#include "LivingRoomLamp.h"
#include "Staircase.h"
#include "NetworkManager.h"
#include "WebDashboard.h"
#include "MqttManager.h"

// ── Modules ──────────────────────────────────────────────
ClimateSensor  climate;
GasSensor      gas;
LivingRoomLamp livingRoom;
Staircase      staircase;
NetworkManager network;
WebDashboard   dashboard;
MqttManager    mqtt;

// ── Timers ───────────────────────────────────────────────
unsigned long lastClimateRead = 0;
unsigned long lastGasRead     = 0;

// Builds the JSON payload sent to every connected browser.
String buildStateJson() {
  String json = "{";

  json += "\"l1\":\""        + livingRoom.stateString() + "\",";
  json += "\"l2\":\""        + staircase.stateString()  + "\",";
  json += "\"mode\":\""      + staircase.modeString()   + "\",";
  json += "\"temp\":\""      + String(climate.temperature(), 1) + "\",";
  json += "\"hum\":\""       + String(climate.humidity(), 1)    + "\",";
  json += "\"gasStatus\":\"" + String(gas.statusString()) + "\",";
  json += "\"gasAlert\":"    + String(gas.alert() ? "true" : "false");

  json += "}";
  return json;
}

// Routes commands from either the LAN WebSocket dashboard or MQTT.
void onCommand(const String& msg) {
  if (msg == "l1_on")                 livingRoom.turnOn();
  else if (msg == "l1_off")           livingRoom.turnOff();
  else if (msg == "stair_manual_on")  staircase.setManual(true);
  else if (msg == "stair_manual_off") staircase.setOff();
  else if (msg == "stair_auto_on")    staircase.setAuto(true);
  else if (msg == "stair_auto_off")   staircase.setOff();

  dashboard.broadcastState();
  mqtt.publishState();
}

void setup() {
  Serial.begin(115200);

  climate.begin();
  gas.begin();
  livingRoom.begin();
  staircase.begin();
  network.begin();
  dashboard.begin(onCommand, buildStateJson);
  mqtt.begin(onCommand, buildStateJson);

  Serial.println("Ready.");
}

void loop() {
  network.loop();
  dashboard.loop();
  mqtt.loop();

  if (staircase.loop()) {
    dashboard.broadcastState();
    mqtt.publishState();
  }

  if (millis() - lastClimateRead > CLIMATE_READ_MS) {
    lastClimateRead = millis();
    if (climate.update()) {
      dashboard.broadcastState();
      mqtt.publishState();
    }
  }

  if (millis() - lastGasRead > GAS_READ_MS) {
    lastGasRead = millis();
    if (gas.update()) {
      dashboard.broadcastState();
      mqtt.publishState();
    }
  }

  delay(10);
}
