#include "ClimateSensor.h"
#include "Config.h"

void ClimateSensor::begin() {
  _dht = new DHT(DHT_PIN, DHT_TYPE);
  _dht->begin();
}

bool ClimateSensor::update() {
  float t = _dht->readTemperature();
  float h = _dht->readHumidity();

  if (!isnan(t) && !isnan(h)) {
    _temp = t;
    _hum  = h;
    return true;
  }
  return false;
}
