#include <Arduino.h>
#include "canController.hpp"
#include "motorController.hpp"

void setup() {
  Serial.begin(115200);

  canController::init();
  motorController::init();
}

void loop() {
  canController::poll();
  motorController::poll();
}