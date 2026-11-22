#pragma once

#include <elapsedMillis.h>
#include "canController.hpp"

// One structure: the motor's state and the operations driving it live together.
// The ops are function pointers; on_state / on_fault take a const Message& so
// they can be handed straight to CANController::requestMessages() (_MB_ptr).
// Implementations are file-static functions in motorController.cpp, assigned
// into these pointers where `motor` is defined.
struct MotorDriver {
    // --- parameter-write handshake ---
    volatile bool paramRecieved = false;
    volatile bool paramOk = false;
    volatile uint16_t expectedAddr = 0;

    // --- live state decoded from the inverter ---
    volatile uint16_t thrvoltage = 0;     // 0x0A3 throttle, 10-bit
    volatile uint8_t current_torque = 0;  // 0x0AC torque feedback
    volatile uint8_t vsmState = 0;        // 0x0AA VSM state
    volatile uint32_t runFault = 0;       // 0x0AB run fault word
    volatile bool lockout = true;
    volatile int32_t veh_speed = 0;

    uint32_t torque = 90;
    const uint8_t iqCurrent = 200;
    const uint8_t idCurrent = 200;
    volatile bool direction = 1;

    elapsedMillis elapsed;

    void (*init)() = nullptr;
    void (*poll)() = nullptr;
    void (*on_state)(const Message& msg) = nullptr;
    void (*on_fault)(const Message& msg) = nullptr;
    // torque / speed / direction / command flags (byte 5) / commanded torque limit.
    void (*send_command)(uint8_t torque, uint8_t speed, uint8_t direction, uint8_t flags,
                         uint8_t torqueLimit) = nullptr;
    void (*throttle_cmd)() = nullptr;
};

// The single motor instance, defined (and wired up) in motorController.cpp.
extern MotorDriver motor;
