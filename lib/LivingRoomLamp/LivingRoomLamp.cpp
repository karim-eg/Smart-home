#include "LivingRoomLamp.h"
#include "Config.h"

void LivingRoomLamp::begin() {
  pinMode(LIVING_ROOM_RELAY_PIN, OUTPUT);
  digitalWrite(LIVING_ROOM_RELAY_PIN, RELAY_OFF);
  _on = false;
}

void LivingRoomLamp::turnOn() {
  digitalWrite(LIVING_ROOM_RELAY_PIN, RELAY_ON);
  _on = true;
}

void LivingRoomLamp::turnOff() {
  digitalWrite(LIVING_ROOM_RELAY_PIN, RELAY_OFF);
  _on = false;
}
