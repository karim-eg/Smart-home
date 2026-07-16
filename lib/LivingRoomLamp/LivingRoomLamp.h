#pragma once
#include <Arduino.h>

// Simple on/off relay-controlled lamp (living room).
class LivingRoomLamp {
public:
  void begin();
  void turnOn();
  void turnOff();

  bool isOn() const { return _on; }
  String stateString() const { return _on ? "ON" : "OFF"; }

private:
  bool _on = false;
};
