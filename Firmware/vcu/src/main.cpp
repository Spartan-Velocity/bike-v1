#include <Arduino.h>
#include "canController.hpp"

void setup() {
  canController::init();
}

void loop() {
  canController::poll();
}