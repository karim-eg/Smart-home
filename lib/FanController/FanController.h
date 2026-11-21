#pragma once
#include <Arduino.h>

class FanController {
public:
  void begin();
  bool autoStartEnabled() const { return _autoStartEnabled; }
  bool setAutoStartEnabled(bool enabled);
  void powerAndSpeed();
  void swing();

private:
  bool _autoStartEnabled = false;
  bool saveSetting() const;
  void sendStartupSequence();
};
