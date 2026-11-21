#pragma once

#include <Arduino.h>

class StorageManager {
public:
  void begin();
  bool sdReady() const { return _sdReady; }
  bool webFilesReady() const { return _webFilesReady; }

private:
  bool _sdReady = false;
  bool _webFilesReady = false;
};

extern StorageManager storage;
