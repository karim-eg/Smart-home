#pragma once
#include <Arduino.h>

// Non-blocking automatic watering controller for the mango plant.
class PlantIrrigation {
public:
  void begin();
  // Returns true when sensor values or watering state changed.
  bool update();

  int rawValue() const { return _rawValue; }
  int moisturePercent() const { return _moisturePercent; }
  bool hasReading() const { return _hasReading; }
  bool pumpOn() const { return _pumpOn; }
  bool soaking() const { return _soaking; }
  bool hasWatered() const { return _hasWatered; }
  unsigned long secondsSinceWatering() const;

private:
  int _rawValue = 0;
  int _moisturePercent = 0;
  bool _hasReading = false;
  bool _pumpOn = false;
  bool _soaking = false;
  bool _hasWatered = false;
  unsigned long _pumpStartedAt = 0;
  unsigned long _wateringFinishedAt = 0;
  unsigned long _lastWateringAt = 0;
  unsigned long _lastReadAt = 0;
};
