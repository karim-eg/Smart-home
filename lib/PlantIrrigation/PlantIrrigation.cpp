#include "PlantIrrigation.h"
#include "Config.h"

void PlantIrrigation::begin() {
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  pinMode(SOIL_PIN, INPUT);
  _lastReadAt = millis();
}

bool PlantIrrigation::update() {
  const unsigned long now = millis();
  bool changed = false;

  if (_pumpOn && now - _pumpStartedAt >= PUMP_TIME_MS) {
    digitalWrite(PUMP_PIN, LOW);
    _pumpOn = false;
    _soaking = true;
    _wateringFinishedAt = now;
    Serial.println("Watering finished.");
    changed = true;
  }

  if (_soaking && now - _wateringFinishedAt >= SOIL_SOAK_MS) {
    _soaking = false;
    Serial.println("Soil soaking finished.");
    changed = true;
  }

  if (now - _lastReadAt >= SOIL_READ_MS) {
    _lastReadAt = now;
    _rawValue = analogRead(SOIL_PIN);
    _moisturePercent = constrain(map(_rawValue, 1023, 0, 0, 100), 0, 100);
    _hasReading = true;
    changed = true;

    if (_rawValue > SOIL_DRY_THRESHOLD && !_pumpOn && !_soaking) {
      Serial.println("Soil is dry. Starting pump...");
      digitalWrite(PUMP_PIN, HIGH);
      _pumpOn = true;
      _pumpStartedAt = now;
      _lastWateringAt = now;
      _hasWatered = true;
    }
  }

  return changed;
}

unsigned long PlantIrrigation::secondsSinceWatering() const {
  return _hasWatered ? (millis() - _lastWateringAt) / 1000UL : 0;
}
