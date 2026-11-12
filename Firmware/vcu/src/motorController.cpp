#include "motorController.hpp"
#include <cstdint>

#include "FlexCAN_T4.h"
#include "canController.hpp"
#include "config.hpp"
#include "elapsedMillis.h"

void motorController::init() {
  Serial.println("Initalizing motor controller");

  canController::requestMessageTemp(motorConfig::MSGID_RX_PARAM, [](const Message& msg) {
    if ((msg.buf[0] | (msg.buf[1] << 8)) != expectedAddr) {
      return;
    }
    paramOk = (msg.buf[2] == 1);
    paramRecieved = true;
  });

  canController::requestMessages(motorConfig::MSGID_RT_STATES, onStateMessage);

  for (const auto& param : motorConfig::PARAMS) {
    Serial.printf("param %u write value %u\n", param[0], param[1]);

    MessageData data =
        CAN_DATA(param[0] & 0xFF, param[0] >> 8, 1, 0, param[1] & 0xFF, param[1] >> 8, 0, 0);

    paramRecieved = false;
    paramOk = false;
    expectedAddr = param[0];

    canController::write(motorConfig::MSGID_TX_PARAM, &data);

    auto timeout = elapsedMillis();
    while (!paramRecieved && timeout < motorConfig::PARAM_RX_TIMEOUT) {
      canController::poll();
    }

    if (!paramRecieved) {
      Serial.printf("param %u write timeout\n", param[0]);
    } else if (!paramOk) {
      Serial.printf("param %u write failed\n", param[0]);
    }
  }

  elapsed = elapsedMillis();
}

void motorController::poll() {
  if (elapsed > motorConfig::COMMAND_MSG_RATE) {
    sendCommandMessage();
    elapsed = elapsedMillis();
  }
}

void motorController::onStateMessage(const Message& msg) {
  bool lockoutStatus = bitRead(msg.buf[6], 7);
  if (lockoutStatus && !lockout) {
    enabled = false;
    lockout = true;

    Serial.println("Motor lockout enabled");
  }
}

void motorController::sendCommandMessage() {
  auto on = enabled && !lockout;
  auto torqueOut = on ? toruqe : 0;
  auto direction = torqueOut > 0;
  MessageData data = CAN_DATA(torqueOut & 0xFF, torqueOut >> 8 & 0xFF, 0, 0, direction, on, 0, 0);
  canController::write(motorConfig::MSGID_TX_COMMAND, &data);
}
