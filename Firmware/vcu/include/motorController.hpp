#pragma once

#include <elapsedMillis.h>
#include "canController.hpp"

class MotorController {
    volatile static inline bool paramRecieved = false;
    volatile static inline bool paramOk = false;
    volatile static inline uint16_t expectedAddr = 0;

    volatile static inline uint16_t thrvoltage = 0;     // 0x0A3 throttle, 10-bit
    volatile static inline uint8_t current_torque = 0;  // 0x0AC torque feedback
    volatile static inline uint8_t vsmState = 0;        // 0x0AA VSM state
    volatile static inline uint32_t runFault = 0;       // 0x0AB run fault word
    volatile static inline bool lockout = true;
    volatile static inline int32_t veh_speed = 0;

    static inline uint32_t torque = 90;
    const static inline uint8_t iqCurrent = 200;
    const static inline uint8_t idCurrent = 200;
    volatile bool direction = 1;

    static elapsedMillis elapsed;

    static void on_state(const Message& msg);
    static void on_fault(const Message& msg);
    static void send_command(uint8_t torque, uint8_t speed, uint8_t direction, uint8_t flags,
                             uint8_t torqueLimit);
    static void throttle_cmd();

  public:
    static void init();
    static void poll();
};

// The single motor instance, defined (and wired up) in motorController.cpp.
// extern MotorDriver motor;
