#pragma once
#include <Arduino.h>
#include <DNSServer.h>

// Brings up WiFi (static IP, STA mode), mDNS, and the local DNS server.
class NetworkManager {
public:
  void begin();
  void loop();

private:
  DNSServer _dnsServer;
};
