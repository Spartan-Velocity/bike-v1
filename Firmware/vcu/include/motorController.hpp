#pragma once

#include <elapsedMillis.h>

#include "canController.hpp"



struct MotorController {
  inline static volatile bool paramRecieved{false};
  inline static volatile bool paramOk{false};
  inline static volatile uint16_t expectedAddr{};
  // Mirrors the inverter lockout state. Starts locked; cleared once the VSM
  // reports it is ready (state 4), re-latched on a lockout/disable (state 7).
  inline static volatile bool lockout{true};
  // Latest decoded throttle reading from the 0x0A3 analog input frame.
  inline static volatile uint16_t throttleVoltage{0};
  inline static elapsedMillis elapsed{};
};


struct Control {
  volatile uint16_t voltage;        
  volatile uint8_t  current_torque;
  volatile uint8_t  vsmState;       
  volatile uint32_t runFault;      
  volatile bool     lockout;
  volatile int32_t veh_speed;

  uint32_t torque;
  const uint8_t iqCurrent = 200;
  const uint8_t idCurrent = 200;
  volatile bool direction = 1;

};

 

static void init();
static void poll();
// torque / speed / direction / command flags (byte 5) / commanded torque limit.
static void sendCommandMessage(uint8_t torque, uint8_t speed, uint8_t direction,
                                uint8_t flags, uint8_t torqueLimit);

static void onStateMessage(const Message& msg);
static void onFaultMessage();
static void onAnalogMessage();
static void throttleCmd();