#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <functional>

// Maintains the device's outbound MQTT connection. MQTT_TOPIC_PREFIX is used
// as the base for ".../state", ".../command", and ".../availability".
class MqttManager {
public:
  using CommandCallback = std::function<void(const String&)>;
  using StateProvider = std::function<String()>;

  void begin(CommandCallback onCommand, StateProvider stateProvider);
  void loop();
  void publishState();

private:
  WiFiClient _networkClient;
  PubSubClient _client{_networkClient};
  CommandCallback _onCommand;
  StateProvider _stateProvider;
  String _stateTopic;
  String _commandTopic;
  String _availabilityTopic;
  unsigned long _lastConnectAttempt = 0;

  bool connect();
  static MqttManager* _instance;
  static void onMessage(char* topic, uint8_t* payload, unsigned int length);
};
