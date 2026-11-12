#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>

#include "canController.hpp"
#include "config.hpp"

void MotorController::init() {
    Serial.println("Initalizing Motor Controller");

    CANController::requestMessageTemp(motorConfig::MSGID_RX_PARAM, [](const CAN_message_t& msg) {
        if ((msg.buf[0] | (msg.buf[1] << 8)) != expectedAddr) {
            return;
        }
        paramOk = (msg.buf[2] == 1);
        paramRecieved = true;
    });

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
            Serial.printf("param %u write timeout\n", param[0]);
        } else if (!paramOk) {
            Serial.printf("param %u write failed\n", param[0]);
        } else {
            Serial.printf("param %u write success, value %u\n", param[0], param[1]);
        }
    }

    Serial.println("Initalized Motor Controller");
}

void MotorController::poll() {}