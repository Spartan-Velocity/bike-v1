#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>
#include <stdint.h>
#include <math.h>

#include "canController.hpp"
#include "config.hpp"

void MotorController::init() {
    Serial.println("Initalizing Motor Controller");

    CANController::requestMessages(motorConfig::MSGID_RX_PARAM, [](const Message& msg) {
        uint16_t addr = readU16LE(&msg.buf[0]);
        uint32_t val = readU32LE(&msg.buf[4]);
        Serial.printf("Param 0x%04X = %lu\n", addr, val);

        if (readU16LE(&msg.buf[0]) != expectedAddr) {
            return;
        }
        paramOk = (msg.buf[2] == 1);
        paramRecieved = true;
    });

    CANController::requestMessages(motorConfig::MSGID_RT_STATES, onState);
    CANController::requestMessages(motorConfig::MSGID_RT_FAULTS, onFault);
    CANController::requestMessages(motorConfig::MSGID_RT_THROTTLE, onThrottle);
    CANController::requestMessages(motorConfig::MSGID_RT_TORQUE, onTorque);
    CANController::requestMessages(motorConfig::MSGID_RT_TORQUE, onTorque);

    for (const auto& param : motorConfig::PARAMS) {
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
            Serial.printf("param %u timeout\n", param[0]);
        } else if (!paramOk) {
            Serial.printf("param %u failed\n", param[0]);
        }
        // else                        Serial.printf("param %u ok\n", param[0]);
    }

    elapsed = 0;
    Serial.println("Initalized Motor Controller");
}

void MotorController::sendCommand(uint8_t torque, uint8_t speed, uint8_t direction, uint8_t flags,
                                  uint8_t torqueLimit) {
    MessageData data = CAN_DATA(torque, 0, speed, 0, direction, flags, torqueLimit, 0);
    CANController::write(motorConfig::MSGID_TX_COMMAND, &data);
}


void MotorController::torqueCmd() {
    const int8_t poles = 20;
    
    uint16_t current = (3*poles) / (4*torque*flux);
    if (current >= (sqrt((iqCurrent * iqCurrent) + (idCurrent * idCurrent))) / 10) {
        torque = 90;
    }

    Serial.println(thrvoltage);
    sendCommand(motorConfig::TEST_TORQUE, 0, 1, 2, 1.5 * motorConfig::TEST_TORQUE);
}


void MotorController::poll() {
    // if (elapsed <= motorConfig::COMMAND_MSG_RATE) return;
    // elapsed = 0;

    // if (lockout) motor_send_command(0, 0, 1, 1, 0);
    // if (thrvoltage > motorConfig::THROTTLE_THRESHOLD) motor_throttle_cmd();
    // else motor_send_command(0, 0, 1, 3, 0);

    switch (vsmState) {
        case 4: {
            sendCommand(0, 0, 1, 3, 0);
            torqueCmd();
        } break;

        case 6: {
            Serial.printf("Drive Enabled");
            // drive();
        } break;

        case 7: {
            MessageData data = CAN_DATA(0x14, 0x00, 1, 0, 0, 0, 0, 0);
            CANController::write(motorConfig::MSGID_TX_PARAM, &data);
        } break;

        default:
            break;
    }


}

void MotorController::onThrottle(const Message& msg) {
    thrvoltage = readU16LE(&msg.buf[0]) & 0x3FF;
}

void MotorController::onTorque(const Message& msg) {
    current_torque = msg.buf[0];
}

void MotorController::onState(const Message& msg) {
    vsmState = msg.buf[0];
}

void MotorController::onFault(const Message& msg) {
    runFault = readU32LE(&msg.buf[4]);

    
    if (!runFault) {
        return;
    }
    if (runFault & 0x00000400) {
        Serial.println("Under Voltage Error");
    }
    if (runFault & 0x00000100) {
        Serial.println("Hardware Gate Fault");
        vsmState = 7;
        lockout = true;
    }

    if (runFault & 0x40000000) {
        Serial.println("Resolver Not Connected");
    }
    if (runFault & 0x00000002) {
        Serial.println("Over-Current Fault");
    }
    if (runFault & 0x00000800) {
        Serial.println("CAN Command Message Lost");
    }
    if (runFault & 0x00000020) {
        Serial.println("Accelerator Input Open");
    }


}