#pragma once

#include <cstdint>
#include "HardwareSerial.h"

namespace canConfig {

constexpr bool canMonitorEnabled = true;

constexpr uint32_t CAN_BAUD_RATE = 500000;
constexpr uint8_t CAN_MAX_MB = 64;

}  // namespace canConfig

namespace wheelConfig {

constexpr uint32_t WHEEL_CONTROLLER_CANID = 0x0A0u;

constexpr uint32_t MSGID_TX_COMMAND = WHEEL_CONTROLLER_CANID + 0x20u;
constexpr uint32_t MSGID_RT_STATES = WHEEL_CONTROLLER_CANID + 0x0Au;
constexpr uint32_t MSGID_RT_FAULTS = WHEEL_CONTROLLER_CANID + 0x0Bu;

constexpr uint16_t COMMAND_MSG_TIMEOUT = 10;

}  // namespace wheelConfig

namespace pumpConfig {

constexpr uint32_t PUMP_PWM_PIN = 28;
constexpr uint32_t FAULT_LED_PIN = 6;

constexpr int PUMP_PWM_FREQUENCY = 1000;  // Max allowed frequency is 1100Hz
constexpr float PUMP_PWM_DUTYCYCLE = 14;  // Min allowed duty cycle is 13%

}  // namespace pumpConfig

namespace displayConfig {

inline auto& NEXTION = Serial4;
constexpr uint32_t DISPLAY_BAUD = 9600;
constexpr uint16_t DISPLAY_TIMEOUT = 5000;

}  // namespace displayConfig