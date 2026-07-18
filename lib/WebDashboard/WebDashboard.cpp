#include "WebDashboard.h"
#include "WebPage.h"
#include "WebScript.h"
#include "WebStyle.h"
#include <Updater.h>

namespace {
const char update_html[] PROGMEM = R"rawliteral(
<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Firmware Update</title><style>body{margin:0;min-height:100vh;display:grid;place-items:center;background:#090d16;color:#edf2f9;font-family:system-ui,sans-serif}.card{width:min(440px,calc(100% - 32px));padding:28px;border:1px solid #273244;border-radius:16px;background:#121926}h1{margin:0 0 8px;font-size:25px}p{color:#aab7c9;line-height:1.5}.file{display:block;margin:20px 0;padding:16px;border:1px dashed #52617a;border-radius:10px}.file input{width:100%;margin-top:10px}button{width:100%;padding:12px;border:0;border-radius:9px;background:#818cf8;color:#fff;font-weight:700;font-size:15px;cursor:pointer}button:disabled{opacity:.55;cursor:wait}#status{min-height:24px;margin:15px 0 0;color:#aab7c9}.back{display:block;margin-top:20px;color:#a5b4fc;text-align:center}</style></head><body><main class="card"><h1>Firmware Update</h1><p>Choose a compiled <code>.bin</code> file to install it. Keep this page open until the device restarts.</p><form id="form" method="POST" action="/update" enctype="multipart/form-data"><label class="file">Firmware file<input id="file" name="firmware" type="file" accept=".bin,application/octet-stream" required></label><button id="submit" type="submit">Install Update</button></form><p id="status" role="status"></p><a class="back" href="/">Back to dashboard</a></main><script>document.getElementById('form').addEventListener('submit',function(e){e.preventDefault();var f=document.getElementById('file').files[0],b=document.getElementById('submit'),s=document.getElementById('status');if(!f)return;s.textContent='Uploading firmware… Do not disconnect power.';b.disabled=true;var x=new XMLHttpRequest;x.open('POST','/update');x.onload=function(){s.textContent=x.responseText;b.disabled=false};x.onerror=function(){s.textContent='Upload failed. Check the device connection.';b.disabled=false};x.send(new FormData(this))})</script></body></html>
)rawliteral";
}

WebDashboard* WebDashboard::_instance = nullptr;

void WebDashboard::begin(WSMessageCallback onMessage, StateProvider stateProvider) {
  _onMessage     = onMessage;
  _stateProvider = stateProvider;
  _instance      = this;

  _server.on("/", [this]() { handleRoot(); });
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
  if (_rebootRequested) {
    delay(250);  // Let the HTTP response reach the browser before rebooting.
    ESP.restart();
  }
}

void WebDashboard::handleRoot() {
  _server.sendHeader("Cache-Control", "no-store, max-age=0");
  _server.send_P(200, PSTR("text/html; charset=utf-8"), index_html);
}

void WebDashboard::handleUpdatePage() {
  _server.sendHeader("Cache-Control", "no-store, max-age=0");
  _server.send_P(200, PSTR("text/html; charset=utf-8"), update_html);
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
