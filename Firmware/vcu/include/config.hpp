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

constexpr uint32_t MSGID_TX_COMMAND = WHEEL_CONTROLLER_CANID + 0x20U;
constexpr uint32_t MSGID_RT_STATES = WHEEL_CONTROLLER_CANID + 0x0AU;
constexpr uint32_t MSGID_RT_FAULTS = WHEEL_CONTROLLER_CANID + 0x0BU;

constexpr uint32_t MSGID_TX_PARAM = WHEEL_CONTROLLER_CANID + 0x21U;
constexpr uint32_t MSGID_RX_PARAM = WHEEL_CONTROLLER_CANID + 0x22U;

// clang-format off

// struct PARAMS{
//   uint16_t 


// }

constexpr uint16_t PARAMS[][2] = {
  {0x0064, 200}, //Iq axis Current Limit x10
  {0x0065, 200}, //Id axis Current Limit x10
  {0x0066, 4000}, //HVDC Limit x10
  {0x0068, 1000}, //HVDC Undervolt x10

  {0x00AD, 2}, // Dischrge Enable
  {0x0080, 100}, // Maxspee RPM
  {0x0081, 1000},// Motor torque_limit x10
  {0x008C, 1}, // Precharge Bypassed
  {0x008E, 0}, // Run_mode (Torque 0 OR Speed 1)
  {0x008F, 0}, // Inv_cmd_mode (CAN 0 OR VSM 1)

  {0x0070, 600}, // Inverter Overtemp x10
  {0x006F, 100}, //Motor_Overspeed RPM
  {0x000C, 29}, //Gamma Adjust x10

  {0x00F1, 12}, //PWM Nominal Freq kHz
  {0x00F6, 12}, // PWM minimum variable Freq kHz
  {0x00F7, 12}, // PWM Max variable Freq kHz


  {0x0014, 0}, // Fault Clear

};

// clang-format on

constexpr uint16_t PARAM_RX_TIMEOUT = 1000;

constexpr uint16_t COMMAND_MSG_RATE = 2;

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