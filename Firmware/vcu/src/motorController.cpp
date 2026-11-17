#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>
#include <cstdint>

#include "canController.hpp"
#include "config.hpp"

void MotorController::init() {
    Serial.println("Initalizing Motor Controller");

    CANController::requestMessageTemp(motorConfig::MSGID_RX_PARAM, [](const Message& msg) {
        if ((msg.buf[0] | (msg.buf[1] << 8)) != expectedAddr) {
            return;
        }
        paramOk = (msg.buf[2] == 1);
        paramRecieved = true;
    });

    CANController::requestMessages(motorConfig::MSGID_RT_STATES, onStateMessage);

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
            Serial.printf("param %u write timeout\n", param[0]);
        } else if (!paramOk) {
            Serial.printf("param %u write failed\n", param[0]);
        } else {
            Serial.printf("param %u write success\n", param[0]);
        }
    }

    CANController::requestMessages(motorConfig::MSGID_RX_THROTTLE, onThrottleMsg);
    CANController::requestMessages(motorConfig::MSGID_RX_TEMP, onTempMsg);
    CANController::requestMessages(motorConfig::MSGID_RX_SPEED, onSpeedMsg);

    elapsed = 0;
    Serial.println("Initalized Motor Controller");
}

void MotorController::onThrottleMsg(const Message& msg) {
    uint16_t throttleInput = ((msg.buf[1] << 8) + msg.buf[0]) & 0x3FF;
}  // TODO: update throttle msg

void MotorController::onTempMsg(const Message& msg) {
    uint16_t motorTemp = (msg.buf[5] << 8) + msg.buf[4];
}  // TODO: update temp msg

void MotorController::onSpeedMsg(const Message& msg) {
    uint16_t motorSpeed = (msg.buf[3] << 8) + msg.buf[2];
}  // TODO: update throttle msg

void MotorController::poll() {
    if (elapsed > motorConfig::COMMAND_MSG_RATE) {
        sendCommandMessage();
        elapsed = 0;
    }
}

void MotorController::onStateMessage(const Message& msg) {
    bool lockoutStatus = bitRead(msg.buf[6], 7);
    if (lockoutStatus && !lockout) {
        enabled = false;
        lockout = true;

        Serial.println("Motor lockout enabled");
    }
}

void MotorController::sendCommandMessage() {
    auto on = enabled && !lockout;
    auto torqueOut = on ? toruqe : 0;
    auto direction = torqueOut > 0;
    MessageData data = CAN_DATA(torqueOut & 0xFF, torqueOut >> 8 & 0xFF, 0, 0, direction, on, 0, 0);
    CANController::write(motorConfig::MSGID_TX_COMMAND, &data);
}
