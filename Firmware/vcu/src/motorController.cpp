#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>

#include "canController.hpp"
#include "config.hpp"
#include "wiring.h"

void MotorController::init() {
    Serial.println("Initalizing Motor Controller");

    CANController::requestMessages(motorConfig::MSGID_RX_PARAM, [](const Message& msg) {
        if ((msg.buf[0] | (msg.buf[1] << 8)) != expectedAddr) {
            return;
        }
        paramOk = (msg.buf[2] == 1);
        paramData = msg.buf[4] | (msg.buf[5] << 8);
        paramRecieved = true;
    });

    CANController::requestMessages(motorConfig::MSGID_RT_STATES, onStateMessage);
    CANController::requestMessages(motorConfig::MSGID_RT_FAULTS, onFaultMessage);

    for (const auto& param : motorConfig::PARAMS) {
        Serial.printf("param %u write value %u\n", param[0], param[1]);

        MessageData data =
            CAN_DATA(param[0] & 0xFF, param[0] >> 8, 1, 0, param[1] & 0xFF, param[1] >> 8, 0, 0);

        paramRecieved = false;
        paramOk = false;
        expectedAddr = param[0];

        CANController::write(motorConfig::MSGID_TX_PARAM, &data);

        elapsedMillis timeout{};
        while (!paramRecieved && timeout < motorConfig::PARAM_RX_TIMEOUT) {
            CANController::poll();
        }

        if (!paramRecieved) {
            Serial.println("param write timeout");
        } else if (!paramOk) {
            Serial.println("param write fail");
        } else {
            Serial.println("param write success");
        }
    }

    elapsed = 0;
    Serial.println("Initalized Motor Controller");
}

void MotorController::poll() {
    if (elapsed > motorConfig::COMMAND_MSG_RATE) {
        sendCommandMessage();
        elapsed = 0;
    }
}

auto MotorController::getEEPROMParam(uint16_t addr) -> uint16_t {
    // byte 2 = 0 -> read command, data bytes ignored
    MessageData data = CAN_DATA(addr & 0xFF, addr >> 8, 0, 0, 0, 0, 0, 0);

    paramRecieved = false;
    paramOk = false;
    paramData = 0;
    expectedAddr = addr;

    CANController::write(motorConfig::MSGID_TX_PARAM, &data);

    elapsedMillis timeout{};
    while (!paramRecieved && timeout < motorConfig::PARAM_RX_TIMEOUT) {
        CANController::poll();
    }

    if (!paramRecieved) {
        Serial.println("param write timeout");
        return 0;
    }
    if (!paramOk) {
        Serial.println("param write fail");
        return 0;
    }

    return paramData;
}

void MotorController::onStateMessage(const Message& msg) {
    bool lockoutStatus = bitRead(msg.buf[6], 7);
    if (lockoutStatus && !lockout) {
        enabled = false;
        lockout = true;

        Serial.println("Motor lockout enabled");
    }
}

void MotorController::onFaultMessage(const Message& msg) {
    if (bitRead(msg.buf[4], 3)) {
        enabled = false;

        MessageData data = CAN_DATA(motorConfig::FAULT_CLEAR_PARAM & 0xFF,
                                    motorConfig::FAULT_CLEAR_PARAM >> 8, 1, 0, 0, 0, 0, 0);

        CANController::write(motorConfig::MSGID_TX_PARAM, &data);
    }
}

void MotorController::sendCommandMessage() {
    auto on = enabled && !lockout;
    auto torqueOut = on ? toruqe : 0;
    auto direction = torqueOut > 0;
    MessageData data = CAN_DATA(torqueOut & 0xFF, torqueOut >> 8 & 0xFF, 0, 0, direction, on, 0, 0);
    CANController::write(motorConfig::MSGID_TX_COMMAND, &data);
}
