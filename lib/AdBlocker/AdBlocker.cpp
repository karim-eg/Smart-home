#include "AdBlocker.h"

#include <ESP8266WiFi.h>
#include <SD.h>
#include <time.h>
#include "Config.h"
#include "StorageManager.h"

namespace {
const IPAddress UPSTREAM_DNS(1, 1, 1, 1);
constexpr uint16_t DNS_PORT = 53;
constexpr uint16_t UPSTREAM_LOCAL_PORT = 53000;
constexpr uint16_t PACKET_SIZE = 512;
constexpr uint32_t QUERY_TIMEOUT_MS = 5000;
constexpr uint32_t STATS_SAVE_INTERVAL_MS = 60000;
constexpr size_t HISTORY_FILE_LIMIT = 32 * 1024;
constexpr uint8_t HISTORY_ITEMS = 30;
constexpr char BLOCKLIST_PATH[] = "/block.txt";
constexpr char STATS_PATH[] = "/dns_stats.csv";
constexpr char HISTORY_PATH[] = "/dns_history.csv";
constexpr char OLD_HISTORY_PATH[] = "/dns_history.old";

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
  _storageReady = storage.sdReady();
  loadDomains();
  loadStats();
  _udp.begin(DNS_PORT);
  _upstreamUdp.begin(UPSTREAM_LOCAL_PORT);
  _lastStatsSaveAt = millis();
}

void AdBlocker::loop() {
  handleUpstreamResponses();
  handleQuery();
  if (_storageReady && _statsDirty && millis() - _lastStatsSaveAt >= STATS_SAVE_INTERVAL_MS) {
    saveStats();
  }
}

void AdBlocker::loadDomains() {
  if (!_storageReady) {
    _domainCount = 0;
  }
  File file = _storageReady ? SD.open(BLOCKLIST_PATH, "r") : File();
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

bool AdBlocker::saveDomains() const {
  if (!_storageReady) return false;
  File file = SD.open(BLOCKLIST_PATH, "w");
  if (!file) return false;
  for (uint8_t i = 0; i < _domainCount; ++i) file.println(_domains[i]);
  file.flush();
  const bool saved = !file.getWriteError();
  file.close();
  return saved;
}

void AdBlocker::loadStats() {
  if (!_storageReady) return;
  File file = SD.open(STATS_PATH, "r");
  if (!file) return;
  String line = file.readStringUntil('\n');
  file.close();
  const int first = line.indexOf(',');
  const int second = first < 0 ? -1 : line.indexOf(',', first + 1);
  if (first < 0 || second < 0) return;
  _queries = strtoul(line.substring(0, first).c_str(), nullptr, 10);
  _blocked = strtoul(line.substring(first + 1, second).c_str(), nullptr, 10);
  _forwarded = strtoul(line.substring(second + 1).c_str(), nullptr, 10);
}

bool AdBlocker::saveStats() {
  if (!_storageReady) return false;
  File file = SD.open(STATS_PATH, "w");
  if (!file) return false;
  file.printf("%lu,%lu,%lu\n", static_cast<unsigned long>(_queries),
              static_cast<unsigned long>(_blocked), static_cast<unsigned long>(_forwarded));
  file.flush();
  const bool saved = !file.getWriteError();
  file.close();
  if (saved) {
    _statsDirty = false;
    _lastStatsSaveAt = millis();
  }
  return saved;
}

void AdBlocker::appendBlockedHistory(const String& host) {
  if (!_storageReady) return;
  File file = SD.open(HISTORY_PATH, FILE_WRITE);
  if (!file) return;
  if (file.size() >= HISTORY_FILE_LIMIT) {
    file.close();
    SD.remove(OLD_HISTORY_PATH);
    if (!SD.rename(HISTORY_PATH, OLD_HISTORY_PATH)) return;
    file = SD.open(HISTORY_PATH, FILE_WRITE);
    if (!file) return;
  }

  const time_t now = time(nullptr);
  const unsigned long epoch = now > 1700000000 ? static_cast<unsigned long>(now) : 0;
  const unsigned long uptimeSeconds = millis() / 1000UL;
  file.printf("%lu,%lu,%s\n", epoch, uptimeSeconds, host.c_str());
  file.close();
}

String AdBlocker::historyJson() const {
  String entries[HISTORY_ITEMS];
  uint8_t next = 0, count = 0;
  const char* paths[] = {OLD_HISTORY_PATH, HISTORY_PATH};
  for (const char* path : paths) {
    File file = _storageReady ? SD.open(path, "r") : File();
    if (!file) continue;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.indexOf(',') < 0) continue;
      entries[next] = line;
      next = (next + 1) % HISTORY_ITEMS;
      if (count < HISTORY_ITEMS) ++count;
    }
    file.close();
  }

  String json = "{\"events\":[";
  const uint8_t start = count == HISTORY_ITEMS ? next : 0;
  for (uint8_t i = 0; i < count; ++i) {
    if (i) json += ',';
    const String& entry = entries[(start + i) % HISTORY_ITEMS];
    const int first = entry.indexOf(',');
    const int second = first < 0 ? -1 : entry.indexOf(',', first + 1);
    if (first < 0 || second < 0) continue;
    json += "{\"epoch\":" + entry.substring(0, first) +
        ",\"uptime\":" + entry.substring(first + 1, second) +
        ",\"domain\":\"" + entry.substring(second + 1) + "\"}";
  }
  json += "]}";
  return json;
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
  if (!_storageReady) {
    error = "بطاقة SD غير متاحة؛ لم يتم حفظ النطاق";
    return false;
  }
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
  if (!saveDomains()) {
    --_domainCount;
    error = "تعذر حفظ القائمة على بطاقة SD";
    return false;
  }
  return true;
}

bool AdBlocker::removeDomain(String domain) {
  if (!_storageReady) return false;
  domain.trim();
  domain.toLowerCase();
  for (uint8_t i = 0; i < _domainCount; ++i) {
    if (_domains[i] == domain) {
      const String removed = _domains[i];
      for (uint8_t j = i + 1; j < _domainCount; ++j) _domains[j - 1] = _domains[j];
      --_domainCount;
      if (saveDomains()) return true;
      for (uint8_t j = _domainCount; j > i; --j) _domains[j] = _domains[j - 1];
      _domains[i] = removed;
      ++_domainCount;
      return false;
    }
  }
  return false;
}

String AdBlocker::statsJson() const {
  String json = "{\"queries\":" + String(_queries) +
      ",\"blocked\":" + String(_blocked) +
      ",\"forwarded\":" + String(_forwarded) +
      ",\"last\":\"" + _lastDomain + "\",\"storage\":" +
      String(_storageReady ? "true" : "false") + ",\"domains\":[";
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
  _statsDirty = true;
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
    _statsDirty = true;
    appendBlockedHistory(host);
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
  _statsDirty = true;
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
