#include "Staircase.h"
#include "Config.h"

void Staircase::begin() {
  pinMode(PIR_PIN, INPUT);
  pinMode(STAIR_LAMP_RELAY_PIN, OUTPUT);
  digitalWrite(STAIR_LAMP_RELAY_PIN, RELAY_OFF);
}

void Staircase::applyMode() {
  if (_mode == StairMode::MANUAL) {
    digitalWrite(STAIR_LAMP_RELAY_PIN, RELAY_ON);
    _lampOn = true;
  } else if (_mode == StairMode::OFF) {
    digitalWrite(STAIR_LAMP_RELAY_PIN, RELAY_OFF);
    _lampOn = false;
  }
  // AUTO mode: relay is driven by loop()/PIR, not here.
}

void Staircase::setManual(bool on) {
  _mode = on ? StairMode::MANUAL : StairMode::OFF;
  applyMode();
}

void Staircase::setAuto(bool on) {
  _mode = on ? StairMode::AUTO : StairMode::OFF;
  if (on) {
    // Auto mode starts with the lamp off and waits for a fresh PIR event.
    // This also clears a lamp that was previously on in manual mode.
    digitalWrite(STAIR_LAMP_RELAY_PIN, RELAY_OFF);
    _lampOn = false;
    _lastMotionTime = 0;
  } else {
    applyMode();
  }
}

void Staircase::setOff() {
  _mode = StairMode::OFF;
  applyMode();
}

bool Staircase::loop() {
  if (_mode != StairMode::AUTO) return false;

  bool changed = false;
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    digitalWrite(STAIR_LAMP_RELAY_PIN, RELAY_ON);
    _lastMotionTime = millis();
    if (!_lampOn) changed = true;
    _lampOn = true;
  }

  if (_lampOn && millis() - _lastMotionTime > STAIR_AUTO_OFF_MS) {
    digitalWrite(STAIR_LAMP_RELAY_PIN, RELAY_OFF);
    _lampOn = false;
    changed = true;
  }

  return changed;
}

String Staircase::stateString() const {
  if (_mode == StairMode::MANUAL) return "ON";
  if (_mode == StairMode::AUTO)   return _lampOn ? "ON" : "AUTO";
  return "OFF";
}

String Staircase::modeString() const {
  switch (_mode) {
    case StairMode::MANUAL: return "manual";
    case StairMode::AUTO:   return "auto";
    default:                return "off";
  }
}
