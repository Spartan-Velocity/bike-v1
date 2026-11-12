#include "pumpController.hpp"

#include "config.hpp"
#include "core_pins.h"

void pumpController::init() {
  pinMode(pumpConfig::PUMP_PWM_PIN, OUTPUT);

  analogWriteResolution(8);
  analogWriteFrequency(pumpConfig::PUMP_PWM_PIN, pumpConfig::PUMP_PWM_FREQUENCY);

  analogWrite(pumpConfig::PUMP_PWM_PIN, pumpConfig::PUMP_PWM_DUTYCYCLE);
}