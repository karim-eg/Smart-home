#pragma once
#include <Arduino.h>

enum class GasSensorStatus {
    Offline,
    Warming,
    Active
};

// Handles the MQ-5 gas sensor: warm-up period, offline detection, and alert state.
class GasSensor {
public:
  void begin();
  bool update();
  const char* statusString() const;

  bool alert() const { return _alert; }
  int analogValue() const { return _analog; }

  enum class GasSensorStatus {
      Offline,
      Warming,
      Active
  };

private:
  unsigned long _bootTime = 0;
  
  GasSensorStatus _status = GasSensorStatus::Warming;

  bool _alert = false;
  int _analog = 0;
};
