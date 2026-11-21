#include "GasSensor.h"
#include "Config.h"

void GasSensor::begin() {
  _bootTime = millis();
}

bool GasSensor::update() {
  int analogValue = analogRead(MQ5_AO_PIN);
  bool warmedUp = millis() - _bootTime > GAS_WARMUP_MS;

  GasSensorStatus newStatus;
  bool newAlert = false;

  if (!warmedUp) {
    newStatus = GasSensorStatus::Warming;
  } else if (analogValue <= GAS_OFFLINE_THRESHOLD) {
    newStatus = GasSensorStatus::Offline;
  } else {
    newStatus = GasSensorStatus::Active;
    newAlert = analogValue >= GAS_ALERT_THRESHOLD;
  }

  bool changed = (_status != newStatus) || (_alert != newAlert) || (_analog != analogValue);
  _status = newStatus;
  _alert = newAlert;
  _analog = analogValue;

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
