#include "MqttManager.h"
#include "Config.h"

MqttManager* MqttManager::_instance = nullptr;

void MqttManager::begin(CommandCallback onCommand, StateProvider stateProvider) {
  _onCommand = onCommand;
  _stateProvider = stateProvider;
  _instance = this;

  _stateTopic = String(MQTT_PREFIX) + "/state";
  _commandTopic = String(MQTT_PREFIX) + "/command";
  _availabilityTopic = String(MQTT_PREFIX) + "/availability";

  _client.setServer(MQTT_HOST, MQTT_PORT);
  _client.setCallback(onMessage);
  _client.setBufferSize(512);
}

void MqttManager::loop() {
  if (WiFi.status() != WL_CONNECTED || strlen(MQTT_HOST) == 0) return;

  if (!_client.connected()) {
    if (millis() - _lastConnectAttempt >= 5000UL) {
      _lastConnectAttempt = millis();
      connect();
    }
    return;
  }
  _client.loop();
}

bool MqttManager::connect() {
  //Serial.printf("Connecting to MQTT broker %s...\n", MQTT_HOST);
  const bool connected = _client.connect(
    MQTT_DEVICE_ID,
    MQTT_USER,
    MQTT_PASS,
    _availabilityTopic.c_str(),
    1,
    true,
    "offline");

  if (!connected) {
    //Serial.printf("MQTT connection failed (rc=%d)\n", _client.state());
    return false;
  }

  _client.publish(_availabilityTopic.c_str(), "online", true);
  _client.subscribe(_commandTopic.c_str(), 1);
  publishState();
  Serial.println("MQTT connected");
  return true;
}

void MqttManager::publishState() {
  if (_client.connected() && _stateProvider) {
    const String state = _stateProvider();
    _client.publish(_stateTopic.c_str(), state.c_str(), true);
  }
}

void MqttManager::onMessage(char* topic, uint8_t* payload, unsigned int length) {
  if (!_instance || String(topic) != _instance->_commandTopic) return;

  String command;
  command.reserve(length);
  for (unsigned int i = 0; i < length; ++i) command += static_cast<char>(payload[i]);
  Serial.println("MQTT command: " + command);
  if (_instance->_onCommand) _instance->_onCommand(command);
}
