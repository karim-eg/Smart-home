#include "WebDashboard.h"
#include "AdBlocker.h"
#include <LittleFS.h>
#include <Updater.h>

namespace {
void sendLittleFSFile(ESP8266WebServer& server, const char* path, const char* contentType) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    server.send(404, "text/plain; charset=utf-8", "الملف غير موجود في LittleFS. ارفع ملفات data باستخدام uploadfs.");
    return;
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.streamFile(file, contentType);
  file.close();
}
}

WebDashboard* WebDashboard::_instance = nullptr;

void WebDashboard::begin(WSMessageCallback onMessage, StateProvider stateProvider, AdBlocker& adBlocker) {
  _onMessage     = onMessage;
  _stateProvider = stateProvider;
  _adBlocker = &adBlocker;
  _instance      = this;

  _server.on("/adblock", HTTP_GET, [this]() { handleAdBlockPage(); });
  _server.on("/api/adblock/stats", HTTP_GET, [this]() { handleAdBlockStats(); });
  _server.on("/api/adblock/domains", HTTP_POST, [this]() { handleAddBlockedDomain(); });
  _server.on("/api/adblock/remove", HTTP_POST, [this]() { handleRemoveBlockedDomain(); });
  _server.on("/update", HTTP_GET, [this]() { handleUpdatePage(); });
  _server.on("/update", HTTP_POST,
    [this]() {
      _server.sendHeader("Connection", "close");
      _server.send(_updateSucceeded ? 200 : 500, "text/plain",
                   _updateSucceeded ? "Update complete. The device is restarting…"
                                    : "Update failed. Please try again.");
      if (_updateSucceeded) _rebootRequested = true;
    },
    [this]() { handleFirmwareUpload(); });
  _server.onNotFound([this]() { _server.send(404, "text/plain; charset=utf-8", "Not found"); });
  _server.serveStatic("/", LittleFS, "/", "no-cache");
  _server.begin();

  _webSocket.begin();
  _webSocket.onEvent(staticWSEvent);
}

void WebDashboard::loop() {
  _server.handleClient();
  _webSocket.loop();
  if (_rebootRequested) {
    delay(250);  // Let the HTTP response reach the browser before rebooting.
    ESP.restart();
  }
}

void WebDashboard::handleUpdatePage() {
  sendLittleFSFile(_server, "/update.html", "text/html; charset=utf-8");
}

void WebDashboard::handleAdBlockPage() {
  sendLittleFSFile(_server, "/adblock.html", "text/html; charset=utf-8");
}

void WebDashboard::handleAdBlockStats() {
  _server.sendHeader("Cache-Control", "no-store, max-age=0");
  _server.send(200, "application/json; charset=utf-8", _adBlocker ? _adBlocker->statsJson() : "{}");
}

void WebDashboard::handleAddBlockedDomain() {
  String error;
  if (!_adBlocker || !_adBlocker->addDomain(_server.arg("domain"), error)) {
    _server.send(400, "text/plain; charset=utf-8", error.length() ? error : "خدمة حجب الإعلانات غير متاحة");
    return;
  }
  _server.send(200, "text/plain; charset=utf-8", "تمت الإضافة");
}

void WebDashboard::handleRemoveBlockedDomain() {
  if (!_adBlocker || !_adBlocker->removeDomain(_server.arg("domain"))) {
    _server.send(404, "text/plain; charset=utf-8", "النطاق غير موجود");
    return;
  }
  _server.send(200, "text/plain; charset=utf-8", "تم الحذف");
}

void WebDashboard::handleFirmwareUpload() {
  HTTPUpload& upload = _server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    _updateSucceeded = false;
    Serial.printf("Firmware update started: %s\n", upload.filename.c_str());
    // ESP8266 requires the available sketch slot size up front.
    const size_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.isRunning() && Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    _updateSucceeded = Update.isRunning() && Update.end(true);
    if (_updateSucceeded) Serial.printf("Firmware update complete: %u bytes\n", upload.totalSize);
    else Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    Serial.println("Firmware update aborted");
  }
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
