#pragma once
#include <Arduino.h>

class FanController {
public:
  void begin();
  bool autoStartEnabled() const { return _autoStartEnabled; }
  void setAutoStartEnabled(bool enabled);
  void powerAndSpeed();
  void swing();

private:
  bool _autoStartEnabled = false;
  void saveSetting() const;
  void sendStartupSequence();
};
