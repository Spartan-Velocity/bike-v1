#include <Arduino.h>
#include "canController.hpp"
#include "motorController.hpp"

void setup() {
  canController::init();
  motorController::init();
}

void loop() {
  canController::poll();
  motorController::poll();
}