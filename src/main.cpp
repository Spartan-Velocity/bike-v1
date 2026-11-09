#include <Arduino.h>
#include "can/canController.hpp"
#include "core_pins.h"
#include "display/displayController.hpp"

// #include "pump/pumpController.hpp"
// #include "wheelController/wheelController.hpp"

void setup() {
  Serial.begin(115200);

  canController::init();
  // wheelController::init();
  // pumpController::init();
  displayController::init();

  delay(500);

  Serial.println("Bike ready");
  displayController::writeVal("loaded", 1);
}

void loop() {
  canController::poll();
  // wheelController::poll();
}