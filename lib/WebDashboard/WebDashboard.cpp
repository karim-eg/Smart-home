#include "WebDashboard.h"
#include "WebPage.h"
#include "WebScript.h"
#include "WebStyle.h"

WebDashboard* WebDashboard::_instance = nullptr;

void WebDashboard::begin(WSMessageCallback onMessage, StateProvider stateProvider) {
  _onMessage     = onMessage;
  _stateProvider = stateProvider;
  _instance      = this;

  _server.on("/", [this]() { handleRoot(); });
  _server.on("/dashboard.css", HTTP_GET, [this]() {
    _server.sendHeader("Cache-Control", "no-store, max-age=0");
    _server.send_P(200, PSTR("text/css; charset=utf-8"), dashboard_css);
  });
  _server.on("/dashboard.js", HTTP_GET, [this]() {
    _server.sendHeader("Cache-Control", "no-store, max-age=0");
    _server.send_P(200, PSTR("application/javascript; charset=utf-8"), dashboard_js);
  });
  _server.begin();

  _webSocket.begin();
  _webSocket.onEvent(staticWSEvent);
}

void WebDashboard::loop() {
  _server.handleClient();
  _webSocket.loop();
}

void WebDashboard::handleRoot() {
  _server.sendHeader("Cache-Control", "no-store, max-age=0");
  _server.send_P(200, PSTR("text/html; charset=utf-8"), index_html);
}

void WebDashboard::broadcastState() {
  if (_stateProvider) {
    String payload = _stateProvider();
    _webSocket.broadcastTXT(payload);
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
