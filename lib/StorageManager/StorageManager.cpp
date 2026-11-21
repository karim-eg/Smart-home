#include "StorageManager.h"

#include <LittleFS.h>
#include <SD.h>
#include "Config.h"

StorageManager storage;

namespace {
bool copyLegacyFile(const char* path) {
  if (SD.exists(path) || !LittleFS.exists(path)) return true;
  File source = LittleFS.open(path, "r");
  File target = SD.open(path, "w");
  if (!source || !target) {
    if (source) source.close();
    if (target) target.close();
    return false;
  }

  uint8_t buffer[128];
  bool ok = true;
  while (source.available()) {
    const size_t count = source.read(buffer, sizeof(buffer));
    if (count && target.write(buffer, count) != count) {
      ok = false;
      break;
    }
  }
  target.flush();
  ok = ok && !target.getWriteError() && target.size() == source.size();
  source.close();
  target.close();
  if (ok) LittleFS.remove(path);
  else SD.remove(path);
  return ok;
}
}

void StorageManager::begin() {
  // GPIO0 is a boot strap pin; keep the SD card deselected during boot.
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);

  _webFilesReady = LittleFS.begin();
  _sdReady = SD.begin(SD_CS_PIN, SPI_QUARTER_SPEED);
  if (_sdReady && _webFilesReady) {
    copyLegacyFile("/block.txt");
    copyLegacyFile("/fan.json");
  }
}
