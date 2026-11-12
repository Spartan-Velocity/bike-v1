#include "motorController.hpp"
#include <cstdint>

#include "FlexCAN_T4.h"
#include "canController.hpp"
#include "config.hpp"
#include "elapsedMillis.h"

void motorController::init() {
  Serial.println("Initalizing motor controller");

  canController::requestMessageTemp(motorConfig::MSGID_RX_PARAM, [](const CAN_message_t& msg) {
    if ((msg.buf[0] | (msg.buf[1] << 8)) != expectedAddr) {
      return;
    }
    paramOk = (msg.buf[2] == 1);
    paramRecieved = true;
  });

  for (const auto& param : motorConfig::PARAMS) {
    Serial.printf("param %u write value %u\n", param[0], param[1]);

    MessageData data = CAN_DATA(param[0] & 0xFF,  // parameter address, little-endian
                                param[0] >> 8,
                                1,                // 1 = write, 0 = read
                                0,                // reserved
                                param[1] & 0xFF,  // data, little-endian
                                param[1] >> 8,
                                0,  // reserved
                                0);

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
}

void motorController::poll() {}