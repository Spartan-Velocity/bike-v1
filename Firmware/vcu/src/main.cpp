#include <Arduino.h>

#include "displayController.hpp"
#include "canController.hpp"
#include "elapsedMillis.h"
#include "motorController.hpp"

void initSerial() {
  auto timeout = elapsedMillis();
  Serial.begin(115200);

  while (!Serial && timeout < 3000) {
  }
  Serial.println("Serial initalized");
}

void setup() {
  initSerial();

  canController::init();
  motorController::init();

  displayController::writeVal("loaded", 1);
}

void loop() {
  canController::poll();
  motorController::poll();
}