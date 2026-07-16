#pragma once
#include <Arduino.h>
#include <DHT.h>

// Wraps the DHT11 temperature/humidity sensor.
class ClimateSensor {
public:
  void begin();

  // Reads the sensor. Returns true if a valid new reading was captured.
  bool update();

  float temperature() const { return _temp; }
  float humidity() const { return _hum; }

private:
  DHT* _dht = nullptr;
  float _temp = 0;
  float _hum  = 0;
};
