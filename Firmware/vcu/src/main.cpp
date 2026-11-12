#include <Arduino.h>
#include "canController.hpp"
#include "elapsedMillis.h"
#include "motorController.hpp"

void initSerial() {
  elapsedMillis timeout{};
  Serial.begin(115200);

  while (!Serial && timeout < 3000) {
  }
  Serial.println("Serial initalized");
}

void setup() {
  initSerial();

  canController::init();
  motorController::init();
}

void loop() {
  canController::poll();
  motorController::poll();
}