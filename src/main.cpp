#include <Arduino.h>
#include "Config.h"
#include "ClimateSensor.h"
#include "GasSensor.h"
#include "LivingRoomLamp.h"
#include "Staircase.h"
#include "NetworkManager.h"
#include "WebDashboard.h"

// ── Modules ──────────────────────────────────────────────
ClimateSensor  climate;
GasSensor      gas;
LivingRoomLamp livingRoom;
Staircase      staircase;
NetworkManager network;
WebDashboard   dashboard;

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

// Routes incoming WebSocket commands from the dashboard to the right module.
void onWSMessage(const String& msg) {
  if (msg == "l1_on")                 livingRoom.turnOn();
  else if (msg == "l1_off")           livingRoom.turnOff();
  else if (msg == "stair_manual_on")  staircase.setManual(true);
  else if (msg == "stair_manual_off") staircase.setOff();
  else if (msg == "stair_auto_on")    staircase.setAuto(true);
  else if (msg == "stair_auto_off")   staircase.setOff();

  dashboard.broadcastState();
}

void setup() {
  Serial.begin(115200);

  climate.begin();
  gas.begin();
  livingRoom.begin();
  staircase.begin();
  network.begin();
  dashboard.begin(onWSMessage, buildStateJson);

  Serial.println("Ready.");
}

void loop() {
  network.loop();
  dashboard.loop();

  if (staircase.loop()) {
    dashboard.broadcastState();
  }

  if (millis() - lastClimateRead > CLIMATE_READ_MS) {
    lastClimateRead = millis();
    if (climate.update()) {
      dashboard.broadcastState();
    }
  }

  if (millis() - lastGasRead > GAS_READ_MS) {
    lastGasRead = millis();
    if (gas.update()) {
      dashboard.broadcastState();
    }
  }

  delay(10);
}
