#include "pumpController.hpp"

#include <core_pins.h>
#include <usb_serial.h>

#include "config.hpp"

void PumpController::init() {
    Serial.println("Initalizing Pump Controller");

    pinMode(pumpConfig::PUMP_PWM_PIN, OUTPUT);

    analogWriteResolution(8);
    analogWriteFrequency(pumpConfig::PUMP_PWM_PIN, pumpConfig::PUMP_PWM_FREQUENCY);

    analogWrite(pumpConfig::PUMP_PWM_PIN, pumpConfig::PUMP_PWM_DUTYCYCLE);

    Serial.println("Initalized Pump Controller");
}