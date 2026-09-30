#include "AdBlocker.h"

#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "Config.h"

namespace {
const IPAddress UPSTREAM_DNS(1, 1, 1, 1);
constexpr uint16_t DNS_PORT = 53;
constexpr uint16_t UPSTREAM_LOCAL_PORT = 53000;
constexpr size_t PACKET_SIZE = 512;
constexpr uint32_t QUERY_TIMEOUT_MS = 5000;

bool validDomain(const String& value) {
  if (value.length() < 3 || value.length() > 100 || value.indexOf('.') < 1) return false;
  bool previousDot = true;
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];
    if (c == '.') {
      if (previousDot || i == value.length() - 1) return false;
      previousDot = true;
    } else if (isalnum(static_cast<unsigned char>(c)) || c == '-') {
      if (previousDot && c == '-') return false;
      previousDot = false;
    } else {
      return false;
    }
  }
  return true;
}

bool readQuestion(const uint8_t* packet, uint16_t length, String& host,
                  uint16_t& questionEnd, uint16_t& queryType) {
  if (length < 17 || packet[4] != 0 || packet[5] == 0) return false;
  uint16_t cursor = 12;
  host = "";
  while (cursor < length) {
    const uint8_t labelLength = packet[cursor++];
    if (labelLength == 0) break;
    if ((labelLength & 0xc0) || labelLength > 63 || cursor + labelLength > length) return false;
    if (host.length()) host += '.';
    for (uint8_t i = 0; i < labelLength; ++i) {
      const char c = static_cast<char>(tolower(packet[cursor++]));
      host += (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '.') ? c : '?';
    }
  }
  if (cursor + 4 > length || !host.length()) return false;
  queryType = (packet[cursor] << 8) | packet[cursor + 1];
  questionEnd = cursor + 4;
  return true;
}

void sendNoData(WiFiUDP& udp, uint8_t* packet, uint16_t length,
                uint16_t questionEnd, IPAddress client, uint16_t clientPort) {
  packet[2] = 0x81;
  packet[3] = 0x80;
  packet[6] = packet[7] = packet[8] = packet[9] = packet[10] = packet[11] = 0;
  udp.beginPacket(client, clientPort);
  udp.write(packet, questionEnd);
  udp.endPacket();
}
}

void AdBlocker::begin() {
  if (!LittleFS.begin()) Serial.println("DNS blocklist storage unavailable");
  loadDomains();
  if (!_udp.begin(DNS_PORT)) Serial.println("DNS server failed to bind port 53");
  else Serial.println("DNS ad blocker listening on port 53");
  if (!_upstreamUdp.begin(UPSTREAM_LOCAL_PORT)) Serial.println("DNS upstream socket failed to start");
}

void AdBlocker::loop() {
  handleUpstreamResponses();
  handleQuery();
}

void AdBlocker::loadDomains() {
  File file = LittleFS.open("/block.txt", "r");
  if (file) {
    while (file.available() && _domainCount < MAX_DOMAINS) {
      String domain = file.readStringUntil('\n');
      domain.trim();
      domain.toLowerCase();
      if (validDomain(domain)) _domains[_domainCount++] = domain;
    }
    file.close();
  }
  if (!_domainCount) {
    const char* defaults[] = {
      "doubleclick.net", "googlesyndication.com", "googleadservices.com",
      "adservice.google.com", "adnxs.com", "ads.yahoo.com", "adform.net",
      "taboola.com", "outbrain.com", "scorecardresearch.com"
    };
    for (const char* domain : defaults) _domains[_domainCount++] = domain;
    saveDomains();
  }
}

void AdBlocker::saveDomains() const {
  File file = LittleFS.open("/block.txt", "w");
  if (!file) return;
  for (uint8_t i = 0; i < _domainCount; ++i) file.println(_domains[i]);
  file.close();
}

bool AdBlocker::isBlocked(const String& host) const {
  for (uint8_t i = 0; i < _domainCount; ++i) {
    const String& domain = _domains[i];
    String suffix = ".";
    suffix += domain;
    if (host == domain || (host.length() > domain.length() && host.endsWith(suffix))) return true;
  }
  return false;
}

bool AdBlocker::addDomain(String domain, String& error) {
  domain.trim();
  domain.toLowerCase();
  if (!validDomain(domain)) {
    error = "اكتب نطاقًا صالحًا مثل ads.example.com";
    return false;
  }
  for (uint8_t i = 0; i < _domainCount; ++i) {
    if (_domains[i] == domain) {
      error = "النطاق موجود بالفعل";
      return false;
    }
  }
  if (_domainCount >= MAX_DOMAINS) {
    error = "القائمة ممتلئة (80 نطاقًا)";
    return false;
  }
  _domains[_domainCount++] = domain;
  saveDomains();
  return true;
}

bool AdBlocker::removeDomain(String domain) {
  domain.trim();
  domain.toLowerCase();
  for (uint8_t i = 0; i < _domainCount; ++i) {
    if (_domains[i] == domain) {
      for (uint8_t j = i + 1; j < _domainCount; ++j) _domains[j - 1] = _domains[j];
      --_domainCount;
      saveDomains();
      return true;
    }
  }
  return false;
}

String AdBlocker::statsJson() const {
  String json = "{\"queries\":" + String(_queries) +
      ",\"blocked\":" + String(_blocked) +
      ",\"forwarded\":" + String(_forwarded) +
      ",\"last\":\"" + _lastDomain + "\",\"domains\":[";
  for (uint8_t i = 0; i < _domainCount; ++i) {
    if (i) json += ',';
    json += '\"';
    json += _domains[i];
    json += '\"';
  }
  json += "]}";
  return json;
}

void AdBlocker::handleQuery() {
  const int packetSize = _udp.parsePacket();
  if (packetSize < 12 || packetSize > PACKET_SIZE) return;
  uint8_t packet[PACKET_SIZE];
  const int length = _udp.read(packet, sizeof(packet));
  if (length < 12) return;

  const IPAddress client = _udp.remoteIP();
  const uint16_t clientPort = _udp.remotePort();
  ++_queries;
  String host;
  uint16_t questionEnd = 0, queryType = 0;
  if (!readQuestion(packet, length, host, questionEnd, queryType)) return;
  _lastDomain = host;

  if (host == "smart-home.local" && queryType == 1) {
    answerAddress(packet, length, questionEnd, client, clientPort, LOCAL_IP);
    return;
  }
  if (isBlocked(host) && (queryType == 1 || queryType == 28)) {
    ++_blocked;
    if (queryType == 1) answerAddress(packet, length, questionEnd, client, clientPort, IPAddress(0, 0, 0, 0));
    else sendNoData(_udp, packet, length, questionEnd, client, clientPort);
    return;
  }

  PendingQuery* slot = nullptr;
  for (uint8_t i = 0; i < MAX_PENDING; ++i) {
    if (_pending[i].active && millis() - _pending[i].startedAt >= QUERY_TIMEOUT_MS)
      _pending[i].active = false;
    if (!_pending[i].active && !slot) slot = &_pending[i];
  }
  if (!slot) return;

  const uint16_t clientId = (packet[0] << 8) | packet[1];
  uint16_t upstreamId = ++_nextRequestId;
  if (!upstreamId) upstreamId = ++_nextRequestId;
  packet[0] = upstreamId >> 8;
  packet[1] = upstreamId & 0xff;
  _upstreamUdp.beginPacket(UPSTREAM_DNS, DNS_PORT);
  _upstreamUdp.write(packet, length);
  if (!_upstreamUdp.endPacket()) return;

  slot->active = true;
  slot->upstreamId = upstreamId;
  slot->clientId = clientId;
  slot->client = client;
  slot->clientPort = clientPort;
  slot->startedAt = millis();
  ++_forwarded;
}

void AdBlocker::handleUpstreamResponses() {
  for (uint8_t i = 0; i < MAX_PENDING; ++i) {
    if (_pending[i].active && millis() - _pending[i].startedAt >= QUERY_TIMEOUT_MS)
      _pending[i].active = false;
  }

  const int packetSize = _upstreamUdp.parsePacket();
  if (packetSize < 12 || packetSize > PACKET_SIZE) return;
  uint8_t packet[PACKET_SIZE];
  const int length = _upstreamUdp.read(packet, sizeof(packet));
  if (length < 12) return;
  const uint16_t responseId = (packet[0] << 8) | packet[1];
  for (uint8_t i = 0; i < MAX_PENDING; ++i) {
    PendingQuery& pending = _pending[i];
    if (!pending.active || pending.upstreamId != responseId) continue;
    packet[0] = pending.clientId >> 8;
    packet[1] = pending.clientId & 0xff;
    _udp.beginPacket(pending.client, pending.clientPort);
    _udp.write(packet, length);
    _udp.endPacket();
    pending.active = false;
    return;
  }
}

void AdBlocker::answerAddress(uint8_t* packet, uint16_t length, uint16_t questionEnd,
                              IPAddress client, uint16_t clientPort, IPAddress address) {
  if (questionEnd + 16 > PACKET_SIZE) return;
  packet[2] = 0x81;
  packet[3] = 0x80;
  packet[6] = 0;
  packet[7] = 1;
  packet[8] = packet[9] = packet[10] = packet[11] = 0;
  uint16_t cursor = questionEnd;
  packet[cursor++] = 0xc0; packet[cursor++] = 0x0c;
  packet[cursor++] = 0; packet[cursor++] = 1;
  packet[cursor++] = 0; packet[cursor++] = 1;
  packet[cursor++] = 0; packet[cursor++] = 0; packet[cursor++] = 0; packet[cursor++] = 30;
  packet[cursor++] = 0; packet[cursor++] = 4;
  packet[cursor++] = address[0]; packet[cursor++] = address[1];
  packet[cursor++] = address[2]; packet[cursor++] = address[3];
  _udp.beginPacket(client, clientPort);
  _udp.write(packet, cursor);
  _udp.endPacket();
}
