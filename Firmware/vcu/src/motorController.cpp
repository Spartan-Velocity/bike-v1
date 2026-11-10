#include "motorController.hpp"
#include <cstdint>

#include "FlexCAN_T4.h"
#include "canController.hpp"
#include "config.hpp"
#include "elapsedMillis.h"

namespace motorController {

namespace {

volatile bool paramRecieved;
volatile bool paramOk;
volatile uint16_t expectedAddr;

}  // namespace

void init() {
  for (const auto& param : motorConfig::PARAMS) {
    canController::MessageData data = CAN_DATA(param[0] & 0xFF,  // parameter address, little-endian
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

    canController::requestMessage(motorConfig::MSGID_RX_PARAM, [](const CAN_message_t& msg) {
      if ((msg.buf[0] | (msg.buf[1] << 8)) != expectedAddr) {
        return;
      }
      paramOk = (msg.buf[2] == 1);
      paramRecieved = true;
    });

    canController::write(motorConfig::MSGID_TX_PARAM, &data, 8);

    auto timeout = elapsedMillis();
    while (!paramRecieved && timeout < motorConfig::PARAM_RX_TIMEOUT) {
    }

    if (!paramRecieved) {
      Serial.printf("param %u write timeout\n", param[0]);
    } else if (!paramOk) {
      Serial.printf("param %u write failed\n", param[0]);
    }
  }
}

void poll() {}

}  // namespace motorController