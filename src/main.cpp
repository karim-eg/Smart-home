#include <Arduino.h>
#include "Config.h"
#include "ClimateSensor.h"
#include "GasSensor.h"
#include "LivingRoomLamp.h"
#include "Staircase.h"
#include "NetworkManager.h"
#include "WebDashboard.h"
#include "AdBlocker.h"
#include "MqttManager.h"
#include "PlantIrrigation.h"
#include "FanController.h"

// ── Modules ──────────────────────────────────────────────
ClimateSensor  climate;
GasSensor      gas;
LivingRoomLamp livingRoom;
Staircase      staircase;
NetworkManager network;
WebDashboard   dashboard;
AdBlocker      adBlocker;
MqttManager    mqtt;
PlantIrrigation plant;
FanController  fan;

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
  json += "\"gasAlert\":"    + String(gas.alert() ? "true" : "false") + ",";
  json += "\"soilReady\":"   + String(plant.hasReading() ? "true" : "false") + ",";
  json += "\"soilMoisture\":" + String(plant.moisturePercent()) + ",";
  json += "\"soilRaw\":"     + String(plant.rawValue()) + ",";
  json += "\"pumpOn\":"      + String(plant.pumpOn() ? "true" : "false") + ",";
  json += "\"soaking\":"     + String(plant.soaking() ? "true" : "false") + ",";
  json += "\"lastWatering\":" + String(plant.secondsSinceWatering());
  json += ",\"fanAutoStart\":" + String(fan.autoStartEnabled() ? "true" : "false");

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
  else if (msg == "fan_auto_start_on")  fan.setAutoStartEnabled(true);
  else if (msg == "fan_auto_start_off") fan.setAutoStartEnabled(false);
  else if (msg == "fan_power_speed")    fan.powerAndSpeed();
  else if (msg == "fan_swing")          fan.swing();

  dashboard.broadcastState();
  mqtt.publishState();
}

void setup() {
  Serial.begin(115200);

  climate.begin();
  gas.begin();
  livingRoom.begin();
  staircase.begin();
  plant.begin();
  network.begin();
  adBlocker.begin();
  fan.begin();
  dashboard.begin(onCommand, buildStateJson, adBlocker);
  mqtt.begin(onCommand, buildStateJson);

  Serial.println("Ready.");
}

void loop() {
  network.loop();
  adBlocker.loop();
  dashboard.loop();
  mqtt.loop();

  if (staircase.loop()) {
    dashboard.broadcastState();
    mqtt.publishState();
  }

  if (plant.update()) {
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
