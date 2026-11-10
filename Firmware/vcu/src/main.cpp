#include <Arduino.h>
#include "canController.hpp"
#include "elapsedMillis.h"
#include "motorController.hpp"
#include "usb_serial.h"

void setup() {
  auto timeout = elapsedMillis();
  Serial.begin(115200);

  while (!Serial && timeout < 3000) {};

  canController::init();
  motorController::init();
}

void loop() {
  canController::poll();
  motorController::poll();
}