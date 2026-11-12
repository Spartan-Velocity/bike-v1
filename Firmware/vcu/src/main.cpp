#include <Arduino.h>

#include <elapsedMillis.h>

#include "canController.hpp"
#include "displayController.hpp"
#include "motorController.hpp"

namespace {

void initSerial() {
    auto timeout = elapsedMillis();
    Serial.begin(115200);

    while (!Serial && timeout < 3000) {
    }
    Serial.println("Serial Initalized");
}

}  // namespace

void setup() {
    initSerial();

    CANController::init();
    MotorController::init();

    DisplayController::writeVal("loaded", 1);
}

void loop() {
    CANController::poll();
    MotorController::poll();
}