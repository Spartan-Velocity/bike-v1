#include <Arduino.h>
#include "canController.hpp"
#include "elapsedMillis.h"
#include "motorController.hpp"
#include "usb_serial.h"

void initSerial() {
  auto timeout = elapsedMillis();
  Serial.begin(115200);

  while (!Serial && timeout < 3000) {
  };
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