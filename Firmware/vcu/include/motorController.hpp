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

    inline static bool enabled{false};
    inline static bool lockout{true};

    inline static int16_t toruqe{};

    inline static elapsedMillis elapsed{};

    static void sendCommandMessage();

    static void onStateMessage(const Message& msg);
};