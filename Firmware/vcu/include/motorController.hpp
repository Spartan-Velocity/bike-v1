#pragma once

#include <elapsedMillis.h>
#include <cstdint>

#include "canController.hpp"

class MotorController {
  public:
    static void init();
    static void poll();

    static auto getEEPROMParam(uint16_t addr) -> uint16_t;

  private:
    inline static volatile bool paramRecieved{false};
    inline static volatile bool paramOk{false};
    inline static volatile uint16_t expectedAddr{};

    inline static volatile uint16_t paramData{};

    inline static bool enabled{false};
    inline static bool lockout{true};

    inline static int16_t toruqe{};

    inline static elapsedMillis elapsed{};

    static void sendCommandMessage();

    static void onStateMessage(const Message& msg);
    static void onFaultMessage(const Message& msg);
};