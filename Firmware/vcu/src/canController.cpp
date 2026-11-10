#include "canController.hpp"
#include <cstdint>

#include "config.hpp"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> canController::getBus() {
  static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> bus;
  return bus;
};

void canController::sniffMessage(const Message& msg) {
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

void canController::requestMessages(uint32_t id, _MB_ptr handler, bool interupt) {
  auto mailbox = FLEXCAN_MAILBOX(usedMailboxes);

  getBus().setMB(mailbox, RX);
  getBus().setMBFilter(mailbox, id);
  getBus().onReceive(mailbox, handler);
  if (interupt) {
    getBus().enableMBInterrupt(mailbox);
  }

  usedMailboxes++;
}

void canController::requestMessage(uint32_t id, _MB_ptr handler) {
  getBus().setMBFilter(MB1, id);
  getBus().onReceive(MB1, handler);

  usedMailboxes++;
}

FLEXCAN_MAILBOX canController::requestWriteMailbox() {
  return FLEXCAN_MAILBOX(usedMailboxes++);
}

void canController::write(uint32_t id, const MessageData* data, uint8_t len) {
  CAN_message_t m;
  m.id = id;
  m.len = len;
  memcpy(m.buf, data, len);
  getBus().write(m);
}

void canController::write(FLEXCAN_MAILBOX mailbox,
                          uint32_t id,
                          const MessageData* data,
                          uint8_t len) {
  CAN_message_t m;
  m.id = id;
  m.len = len;
  memcpy(m.buf, data, len);
  getBus().write(mailbox, m);
}

void canController::init() {
  getBus().begin();

  getBus().setBaudRate(canConfig::CAN_BAUD_RATE);
  getBus().setMBFilter(REJECT_ALL);  // disable all mailboxes

  getBus().setMB(MB0, TX);  // set mailbox 0 to transmit default
  usedMailboxes++;

  getBus().setMB(MB1, RX);  // set mailbox 1 to receive default
  getBus().enableMBInterrupt(MB1);
  usedMailboxes++;

  // enable fifo mailbox for message sniffing
  getBus().enableFIFO();
  getBus().setFIFOFilter(ACCEPT_ALL);
  getBus().onReceive(FIFO, sniffMessage);
  // getBus().enableFIFOInterrupt();

  // pass dedicated mailbox messages to fifo for debugging
  getBus().distribute(canConfig::canMonitorEnabled);
}

void canController::poll() {
  getBus().events();
}