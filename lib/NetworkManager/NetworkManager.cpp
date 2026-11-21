#include "NetworkManager.h"
#include "Config.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

void NetworkManager::begin() {
  WiFi.mode(WIFI_STA);

  WiFi.config(LOCAL_IP, GATEWAY_IP, SUBNET_MASK);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");

  MDNS.begin("smart-home");
}

void NetworkManager::loop() {
  if (WiFi.status() == WL_CONNECTED) {
    MDNS.update();
  }
}
