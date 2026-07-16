#pragma once
#include <Arduino.h>

enum class StairMode { OFF, MANUAL, AUTO };

// Staircase lamp: manual override, PIR-driven auto mode (with timeout), or off.
class Staircase {
public:
  void begin();

  // Call every loop() iteration. Returns true if state changed
  // (i.e. something worth broadcasting happened).
  bool loop();

  void setManual(bool on);
  void setAuto(bool on);
  void setOff();

  String stateString() const; // "ON" | "OFF" | "AUTO"
  String modeString() const;  // "manual" | "auto" | "off"

private:
  StairMode _mode = StairMode::OFF;
  bool _lampOn = false;
  unsigned long _lastMotionTime = 0;

  void applyMode();
};
