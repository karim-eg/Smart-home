#include "FanController.h"
#include <IRremote.hpp>
#include <LittleFS.h>
#include "Config.h"

namespace {
const uint16_t powerRaw[] = {
  1230,420, 1230,470, 380,1270, 1230,420, 1230,470, 380,1270,
  380,1270, 380,1320, 380,1270, 380,1270, 1230,420, 380,7970,
  1230,420, 1230,420, 430,1270, 1180,470, 1230,420, 380,1270,
  430,1270, 380,1270, 380,1320, 380,1270, 1230,420, 380
};
const uint16_t swingRaw[] = {
  1230,420, 1230,420, 430,1270, 1230,420, 1230,420, 430,1270,
  380,1270, 380,1320, 380,1270, 380,1270, 380,1320, 1180,7120,
  1230,420, 1230,420, 430,1270, 1230,420, 1230,420, 380,1270,
  430,1270, 380,1270, 380,1320, 380,1270, 380,1270, 1230
};
constexpr uint8_t POWER_LEN = sizeof(powerRaw) / sizeof(powerRaw[0]);
constexpr uint8_t SWING_LEN = sizeof(swingRaw) / sizeof(swingRaw[0]);
constexpr char SETTINGS_PATH[] = "/fan.json";
}

void FanController::begin() {
  File settings = LittleFS.open(SETTINGS_PATH, "r");
  if (settings) {
    String value = settings.readString();
    settings.close();
    _autoStartEnabled = value.indexOf("\"autoStart\":true") >= 0;
  }

  IrSender.begin(FAN_IR_PIN);
  if (_autoStartEnabled) sendStartupSequence();
}

void FanController::setAutoStartEnabled(bool enabled) {
  if (_autoStartEnabled == enabled) return;
  _autoStartEnabled = enabled;
  saveSetting();
}

void FanController::saveSetting() const {
  File settings = LittleFS.open(SETTINGS_PATH, "w");
  if (!settings) {
    Serial.println("Could not save fan settings");
    return;
  }
  settings.printf("{\"autoStart\":%s}", _autoStartEnabled ? "true" : "false");
  settings.close();
}

void FanController::powerAndSpeed() {
  IrSender.sendRaw(powerRaw, POWER_LEN, 38);
}

void FanController::swing() {
  IrSender.sendRaw(swingRaw, SWING_LEN, 38);
}

void FanController::sendStartupSequence() {
  powerAndSpeed();
  delay(300);
  powerAndSpeed();
  delay(300);
  swing();
  delay(300);
}
