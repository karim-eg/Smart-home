#pragma once
#include <Arduino.h>

// Handles the MQ-5 gas sensor: warm-up period, offline detection, and alert state.
class GasSensor {
public:
  void begin();

  bool update();

  String statusString() const { return _status; } // "warming" | "active" | "offline"
  bool alert() const { return _alert; }

private:
  unsigned long _bootTime = 0;
  String _status = "warming";
  bool _alert = false;
};
