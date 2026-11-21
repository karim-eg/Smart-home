#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

// DNS ad blocking, domain-list persistence, and dashboard statistics.
class AdBlocker {
public:
  void begin();
  void loop();
  String statsJson() const;
  String historyJson() const;
  bool addDomain(String domain, String& error);
  bool removeDomain(String domain);

private:
  static constexpr uint8_t MAX_DOMAINS = 80;
  static constexpr uint8_t MAX_PENDING = 4;
  struct PendingQuery {
    bool active = false;
    uint16_t upstreamId = 0;
    uint16_t clientId = 0;
    IPAddress client;
    uint16_t clientPort = 0;
    uint32_t startedAt = 0;
  };
  WiFiUDP _udp;
  WiFiUDP _upstreamUdp;
  String _domains[MAX_DOMAINS];
  uint8_t _domainCount = 0;
  uint32_t _queries = 0;
  uint32_t _blocked = 0;
  uint32_t _forwarded = 0;
  String _lastDomain = "-";
  bool _storageReady = false;
  bool _statsDirty = false;
  uint32_t _lastStatsSaveAt = 0;
  PendingQuery _pending[MAX_PENDING];
  uint16_t _nextRequestId = 0;

  void loadDomains();
  bool saveDomains() const;
  void loadStats();
  bool saveStats();
  void appendBlockedHistory(const String& host);
  bool isBlocked(const String& host) const;
  void handleQuery();
  void handleUpstreamResponses();
  void answerAddress(uint8_t* packet, uint16_t length, uint16_t questionEnd,
                     IPAddress client, uint16_t clientPort, IPAddress address);
};
