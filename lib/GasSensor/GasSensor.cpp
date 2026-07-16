#include "GasSensor.h"
#include "Config.h"

void GasSensor::begin() {
  pinMode(MQ5_DO_PIN, INPUT);
  _bootTime = millis();
}

bool GasSensor::update() {
  int analogVal = analogRead(MQ5_AO_PIN);
  bool warmedUp = (millis() - _bootTime) > GAS_WARMUP_MS;

  String newStatus;
  bool newAlert = false;

  if (analogVal < GAS_OFFLINE_THRESHOLD) {
    newStatus = "offline";
  } else if (!warmedUp) {
    newStatus = "warming";
  } else {
    newStatus = "active";
    newAlert = (digitalRead(MQ5_DO_PIN) == HIGH);
  }

  if (newStatus != _status || newAlert != _alert) {
    _status = newStatus;
    _alert  = newAlert;
    return true;
  }
  return false;
}
