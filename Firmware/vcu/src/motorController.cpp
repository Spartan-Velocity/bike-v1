#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>

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
    CANController::requestMessages(motorConfig::MSGID_RT_FAULTS, onFaultMessage);
    CANController::requestMessages(motorConfig::MSGID_RT_ANALOG, onAnalogMessage);

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

    elapsed = 0;
    Serial.println("Initalized Motor Controller");
}

void MotorController::poll() {
   
    if (elapsed <= motorConfig::COMMAND_MSG_RATE) {
        return;
    }
    elapsed = 0;

    if (lockout) {
        
        sendCommandMessage(0, 0, 1, 1, 0);

    } else if (throttleVoltage > motorConfig::THROTTLE_THRESHOLD) {
        throttleCmd(throttleVoltage);
    
    

    } else {
      
        sendCommandMessage(0, 0, 1, 1, 0);
    }


}

void MotorController::onStateMessage(const Message& msg) {
    switch (msg.buf[0]) {
        case 6:
            Serial.println("Motor Enabled");
            break;

        case 4:
            if (lockout) {
                Serial.println("Lock Removed");
                sendCommandMessage(0, 0, 1, 0, 0);
                lockout = false;
            }
            break;

        case 7:
            lockout = true;
            break;
    }
}

void MotorController::onFaultMessage(const Message& msg) {
    uint32_t run = msg.buf[4] | (static_cast<uint32_t>(msg.buf[5]) << 8) |
                   (static_cast<uint32_t>(msg.buf[6]) << 16) |
                   (static_cast<uint32_t>(msg.buf[7]) << 24);

    if (run & 0x00000100) {
        Serial.println("Hardware Gate Fault");
    }
    if (run & 0x40000000) {
        Serial.println("Resolver Not Connected");
    }
}

void MotorController::throttleCmd(const uint16_t voltage) {
    sendCommandMessage(motorConfig::TEST_TORQUE, 0, 1, 2, (1.5*motorConfig::TEST_TORQUE));
}

void MotorController::onAnalogMessage(const Message& msg) {
    throttleVoltage = msg.buf[0] | ((msg.buf[1] & 0x03) << 8);

}


void MotorController::sendCommandMessage(uint8_t torque, uint8_t speed, uint8_t direction, uint8_t flags, uint8_t torqueLimit) {
    MessageData data = CAN_DATA(torque, 0, speed, 0, direction, flags, torqueLimit, 0);
    CANController::write(motorConfig::MSGID_TX_COMMAND, &data);
}
