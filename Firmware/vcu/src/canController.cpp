#include "canController.hpp"
#include <cstdint>

#include "config.hpp"

namespace canController {

namespace {

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> canBus;

uint8_t usedMailboxes = 0;

void sniffMessage(const Message& msg);

void sniffMessage(const Message& msg) {
  Serial.printf("%10lu %c %03lX [%u]",
                micros(),
                msg.flags.extended ? 'X' : 'S',
                static_cast<uint32_t>(msg.id),
                msg.len);
  for (uint8_t i = 0; i < msg.len; i++) {
    Serial.printf(" %02X", msg.buf[i]);
  }
  Serial.println();
}

}  // namespace

void requestMessages(uint32_t id, _MB_ptr handler, bool interupt) {
  if (usedMailboxes >= canConfig::CAN_MAX_MB - 1) {
    return;
  }
  auto mailbox = FLEXCAN_MAILBOX(usedMailboxes);

  canBus.setMB(mailbox, RX);
  canBus.setMBFilter(mailbox, id);
  canBus.onReceive(mailbox, handler);
  if (interupt) {
    canBus.enableMBInterrupt(mailbox);
  }

  usedMailboxes++;
}

void write(const Message& msg) {
  canBus.write(msg);
}

void write(FLEXCAN_MAILBOX mailbox, uint32_t id, const uint8_t* data, uint8_t len) {
  CAN_message_t m;
  m.id = id;
  m.len = len;
  m.flags.extended = false;
  memcpy(m.buf, data, len);
  canBus.write(mailbox, m);
}

void init() {
  canBus.begin();

  canBus.setBaudRate(canConfig::CAN_BAUD_RATE);
  canBus.setMBFilter(REJECT_ALL);  // disable all mailboxes

  canBus.setMB(MB0, TX);  // set mailbox 0 to transmit default
  usedMailboxes++;

  // enable fifo mailbox for message sniffing
  canBus.enableFIFO();
  canBus.setFIFOFilter(ACCEPT_ALL);
  canBus.onReceive(FIFO, sniffMessage);
  // canBus.enableFIFOInterrupt();

  // pass dedicated mailbox messages to fifo for debugging
  canBus.distribute(canConfig::canMonitorEnabled);
}

void poll() {
  canBus.events();
}

}  // namespace canController