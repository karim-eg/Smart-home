#pragma once
#include <Arduino.h>

// Brings up WiFi (static IP, STA mode) and mDNS. AdBlocker owns DNS port 53.
class NetworkManager {
public:
  void begin();
  void loop();
};
