#pragma once

#include <stdint.h>
#include "canController.hpp"

class MotorController {
  public:
    static void init();

    static void poll();

  private:
    inline static volatile bool paramRecieved{};
    inline static volatile bool paramOk{};
    inline static volatile uint16_t expectedAddr{};

    static void onThrottleMsg(const Message& msg);
    static void onTempMsg(const Message& msg);
    static void onSpeedMsg(const Message& msg);
};