#pragma once

#include <elapsedMillis.h>

#include "canController.hpp"

class MotorController {
  public:
    static void init();
    static void poll();

  private:
    inline static volatile bool paramRecieved{false};
    inline static volatile bool paramOk{false};
    inline static volatile uint16_t expectedAddr{};

    // Mirrors the inverter lockout state. Starts locked; cleared once the VSM
    // reports it is ready (state 4), re-latched on a lockout/disable (state 7).
    inline static volatile bool lockout{true};

    // Latest decoded throttle reading from the 0x0A3 analog input frame.
    inline static volatile uint16_t throttleVoltage{0};

    inline static elapsedMillis elapsed{};

    // torque / speed / direction / command flags (byte 5) / commanded torque limit.
    static void sendCommandMessage(uint8_t torque, uint8_t speed, uint8_t direction,
                                   uint8_t flags, uint8_t torqueLimit);

    static void onStateMessage(const Message& msg);
    static void onFaultMessage(const Message& msg);
    static void onAnalogMessage(const Message& msg);
    static void throttleCmd(const uint16_t torque);
};
