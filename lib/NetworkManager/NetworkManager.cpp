#include "NetworkManager.h"
#include "Config.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

void NetworkManager::begin() {
  WiFi.mode(WIFI_STA);

  if (!WiFi.config(LOCAL_IP, GATEWAY_IP, SUBNET_MASK)) {
    Serial.println("Static IP failed");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected: " + WiFi.localIP().toString());

  if (!MDNS.begin("smart-home")) {
    Serial.println("mDNS failed");
  }
}

void NetworkManager::loop() {
}
