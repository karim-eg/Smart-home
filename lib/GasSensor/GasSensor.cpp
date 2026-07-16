#include "GasSensor.h"
#include "Config.h"

void GasSensor::begin() {
  pinMode(MQ5_DO_PIN, INPUT);
  pinMode(MQ5_AO_PIN, INPUT);
  _bootTime = millis();
}

bool GasSensor::update() {
  _analog = analogRead(MQ5_AO_PIN);
  bool doState = digitalRead(MQ5_DO_PIN);
  
  bool warmedUp = millis() - _bootTime > GAS_WARMUP_MS;

  GasSensorStatus newStatus;
  bool newAlert = false;

  if (!warmedUp) {
    newStatus = GasSensorStatus::Warming;

  } else if (_analog < GAS_OFFLINE_THRESHOLD) {
    newStatus = GasSensorStatus::Offline;

  } else {
    newStatus = GasSensorStatus::Active;
    newAlert = doState;
  }

  bool changed = (_status != newStatus) || (_alert != newAlert);
  _status = newStatus;
  _alert = newAlert;

  return changed;
}

const char* GasSensor::statusString() const {
  switch (_status) {
    case GasSensorStatus::Offline:
      return "offline";

    case GasSensorStatus::Warming:
      return "warming";

    case GasSensorStatus::Active:
      return "active";
  }

  return "unknown";
}