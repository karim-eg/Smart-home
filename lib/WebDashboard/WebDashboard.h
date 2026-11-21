#pragma once
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <functional>

class AdBlocker;

using WSMessageCallback = std::function<void(const String&)>;
using StateProvider     = std::function<String()>;

// Serves the dashboard page over HTTP and pushes/receives live state over WebSocket.
class WebDashboard {
public:
  // onMessage   - called for every text message received from a browser client.
  // stateProvider - called whenever we need the latest state JSON to send out
  //                 (on new client connect, or when you call broadcastState()).
  void begin(WSMessageCallback onMessage, StateProvider stateProvider, AdBlocker& adBlocker);

  void loop();
  void broadcastState();

private:
  ESP8266WebServer  _server{80};
  WebSocketsServer  _webSocket{81};
  WSMessageCallback _onMessage;
  StateProvider     _stateProvider;
  AdBlocker*        _adBlocker = nullptr;

  void handleUpdatePage();
  void handleAdBlockPage();
  void handleAdBlockStats();
  void handleAdBlockHistory();
  void handleAddBlockedDomain();
  void handleRemoveBlockedDomain();
  void handleCommand();
  void handleFirmwareUpload();
  void handleWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len);

  bool _updateSucceeded = false;
  bool _rebootRequested = false;

  // WebSocketsServer wants a plain function pointer, so we trampoline
  // through a single static instance back into the member function.
  static WebDashboard* _instance;
  static void staticWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len);
};
