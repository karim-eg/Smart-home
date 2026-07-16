#include "WebDashboard.h"
#include "WebPage.h"

WebDashboard* WebDashboard::_instance = nullptr;

void WebDashboard::begin(WSMessageCallback onMessage, StateProvider stateProvider) {
  _onMessage     = onMessage;
  _stateProvider = stateProvider;
  _instance      = this;

  _server.on("/", [this]() { handleRoot(); });
  _server.begin();

  _webSocket.begin();
  _webSocket.onEvent(staticWSEvent);
}

void WebDashboard::loop() {
  _server.handleClient();
  _webSocket.loop();
}

void WebDashboard::handleRoot() {
  _server.sendHeader("Content-Type", "text/html; charset=utf-8");
  _server.send(200, "text/html", index_html);
}

void WebDashboard::broadcastState() {
  if (_stateProvider) {
    _webSocket.broadcastTXT(_stateProvider());
  }
}

void WebDashboard::staticWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (_instance) _instance->handleWSEvent(num, type, payload, len);
}

void WebDashboard::handleWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type == WStype_CONNECTED) {
    Serial.println("Client connected");
    broadcastState();
    return;
  }
  if (type != WStype_TEXT) return;

  String msg = String((char*)payload);
  Serial.println("WS msg: " + msg);

  if (_onMessage) _onMessage(msg);
}
