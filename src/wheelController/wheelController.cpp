#include "wheelController/wheelController.hpp"

#include "can/canController.hpp"
#include "config.hpp"
#include "elapsedMillis.h"
#include "util.hpp"

namespace wheelController {

namespace {

bool lockoutEnabled = true;
bool controllerEnabled = false;
Direction direction = forward;
int16_t commandedTorque = 0;

elapsedMillis controlTimeout;

void onStateMessage(const canController::Message& msg);
void onFaultMessage(const canController::Message& msg);

void writeCommandMessage();

void onStateMessage(const canController::Message& msg) {
  bool lockoutStatus = bitRead(msg.buf[6], 7);
  if (lockoutStatus && !lockoutEnabled) {
    controllerEnabled = false;
    lockoutEnabled = true;
  }
}

void onFaultMessage(const canController::Message& msg) {
  // auto postFaults = msg.buf[0] | (msg.buf[1] << 8) | (msg.buf[2] << 16) | (msg.buf[3] << 24);

  // auto runFaults = msg.buf[4] | (msg.buf[5] << 8) | (msg.buf[6] << 16) | (msg.buf[7] << 24);
}

void writeCommandMessage() {
  canController::Message msg{};

  msg.id = wheelConfig::MSGID_TX_COMMAND;
  msg.len = 8;

  auto enabled = (!lockoutEnabled && controllerEnabled);

  // if controller is in lockout or disabled, set torque to 0
  int16_t torqueOut = enabled ? commandedTorque : 0;
  msg.buf[0] = torqueOut & 0xFF;
  msg.buf[1] = (torqueOut >> 8) & 0xFF;

  // speed control - unused
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  msg.buf[4] = direction;
  msg.buf[5] = enabled ? 0x01 : 0x00;

  // torque limit override - unused
  msg.buf[6] = 0;
  msg.buf[7] = 0;

  canController::write(msg);
}

}  // namespace

void init() {
  canController::requestMessages(wheelConfig::MSGID_RT_STATES, onStateMessage);
  canController::requestMessages(wheelConfig::MSGID_RT_FAULTS, onFaultMessage);

  controlTimeout = elapsedMillis();
}

void poll() {
  if (controlTimeout >= wheelConfig::COMMAND_MSG_TIMEOUT) {
    writeCommandMessage();
    controlTimeout = elapsedMillis();
  }
}

void setEnabled(bool enabled) {
  controllerEnabled = enabled;
}

void setTorque(int16_t torque) {
  commandedTorque = torque;
}

}  // namespace wheelController

// TODO: swordpartee - fault reporting