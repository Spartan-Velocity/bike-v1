#pragma once

#include <HardwareSerial.h>

namespace canConfig {

constexpr bool canMonitorEnabled = true;

constexpr uint32_t CAN_BAUD_RATE = 500000;
constexpr uint8_t CAN_MAX_MB = 64;

constexpr uint8_t FIFO_MAILBOX_COUNT = 8;

}  // namespace canConfig

namespace motorConfig {

constexpr uint32_t WHEEL_CONTROLLER_CANID = 0x0A0U;

constexpr uint32_t MSGID_TX_COMMAND = WHEEL_CONTROLLER_CANID + 0x20U;  // 0x0C0 heartbeat
constexpr uint32_t MSGID_RT_ANALOG = WHEEL_CONTROLLER_CANID + 0x03U;   // 0x0A3 throttle input
constexpr uint32_t MSGID_RT_STATES = WHEEL_CONTROLLER_CANID + 0x0AU;   // 0x0AA VSM states
constexpr uint32_t MSGID_RT_FAULTS = WHEEL_CONTROLLER_CANID + 0x0BU;   // 0x0AB fault codes
constexpr uint32_t MSGID_RT_THROTTLE = WHEEL_CONTROLLER_CANID + 0x03U;   // 0x0AB fault codes
constexpr uint32_t MSGID_RT_TORQUE = WHEEL_CONTROLLER_CANID + 0x0CU;  // 0x0AC torque feedback

constexpr uint32_t MSGID_TX_PARAM = WHEEL_CONTROLLER_CANID + 0x21U;  // 0x0C1 param request
constexpr uint32_t MSGID_RX_PARAM = WHEEL_CONTROLLER_CANID + 0x22U;  // 0x0C2 param response

// clang-format off

// struct PARAMS{
//   uint16_t 


// }

constexpr uint16_t PARAMS[][2] = {
    {0x0064, 200}, //Iq axis Current Limit x10
    {0x0065, 200}, //Id axis Current Limit x10
    {0x0066, 4000}, //HVDC Limit x10
    {0x0068, 900}, //HVDC Undervolt x10

    {0x00AD, 2}, // Dischrge Enable
    {0x006F, 50}, // Motor_Overspeed RPM
    {0x0080, 50}, // Maxspeed RPM

    {0x0081, 120}, // Motor torque_limit N*m x10
    {0x008C, 1}, // Precharge Bypassed (0 NO OR 1 YES)
    {0x008E, 0}, // Run_mode (Torque 0 OR Speed 1)
    {0x008F, 0}, // Inv_cmd_mode (CAN 0 OR VSM 1)

    {0x0070, 500}, // Inverter Overtemp x10
    {0x0098, 629}, // Gamma Adjust x10

    {0x00F1, 12}, //PWM Nominal Freq kHz
    {0x00F6, 12}, // PWM minimum variable Freq kHz
    {0x00F7, 12}, // PWM Max variable Freq kHz
  };

// clang-format on

constexpr uint16_t FAULT_CLEAR_ADDR = 0x0014;

constexpr uint16_t PARAM_RX_TIMEOUT = 1000;

// Heartbeat (0x0C0) period in ms. Spec: send every 10ms, state-agnostic.
constexpr uint16_t COMMAND_MSG_RATE = 10;

// Throttle (0x0A3) raw-count threshold above which a test torque is commanded.
constexpr uint16_t THROTTLE_THRESHOLD = 175;

// Fixed torque command used for the bench test (Nm x10, low byte only).
constexpr uint8_t TEST_TORQUE = 200;

}  // namespace motorConfig

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

static inline auto readU16LE(const uint8_t* p) -> uint16_t {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

static inline auto readU32LE(const uint8_t* p) -> uint32_t {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}