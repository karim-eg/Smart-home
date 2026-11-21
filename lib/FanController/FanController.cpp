#include "FanController.h"
#include <IRremote.hpp>
#include <SD.h>
#include "Config.h"
#include "StorageManager.h"

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
  File settings = storage.sdReady() ? SD.open(SETTINGS_PATH, "r") : File();
  if (settings) {
    String value = settings.readString();
    settings.close();
    _autoStartEnabled = value.indexOf("\"autoStart\":true") >= 0;
  }

  IrSender.begin(FAN_IR_PIN);
  if (_autoStartEnabled) sendStartupSequence();
}

bool FanController::setAutoStartEnabled(bool enabled) {
  if (_autoStartEnabled == enabled) return true;
  const bool previous = _autoStartEnabled;
  _autoStartEnabled = enabled;
  if (saveSetting()) return true;
  _autoStartEnabled = previous;
  return false;
}

bool FanController::saveSetting() const {
  if (!storage.sdReady()) return false;
  File settings = SD.open(SETTINGS_PATH, "w");
  if (!settings) return false;
  settings.printf("{\"autoStart\":%s}", _autoStartEnabled ? "true" : "false");
  settings.flush();
  const bool saved = !settings.getWriteError();
  settings.close();
  return saved;
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
