#include "canController.hpp"
#include <cstdint>

#include "FlexCAN_T4.h"
#include "config.hpp"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>& canController::getBus() {
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

void canController::requestMessages(uint32_t id, _MB_ptr handler) {
  auto mailbox = FLEXCAN_MAILBOX(usedMailboxes);

  getBus().setMB(mailbox, RX);
  getBus().setMBFilter(mailbox, id);
  getBus().onReceive(mailbox, handler);
  getBus().enableMBInterrupt(mailbox);

  usedMailboxes++;
}

void canController::requestMessageTemp(uint32_t id, _MB_ptr handler) {
  getBus().setMBFilter(tempMailbox, id);
  getBus().onReceive(tempMailbox, handler);
}

FLEXCAN_MAILBOX canController::requestWriteMailbox() {
  return FLEXCAN_MAILBOX(usedMailboxes++);
}

void canController::write(uint32_t id, const MessageData* data) {
  CAN_message_t m;
  m.id = id;
  m.len = 8;
  memcpy(m.buf, data, 8);
  getBus().write(m);
}

void canController::write(FLEXCAN_MAILBOX mailbox, uint32_t id, const MessageData* data) {
  CAN_message_t m;
  m.id = id;
  m.len = 8;
  memcpy(m.buf, data, 8);
  getBus().write(mailbox, m);
}

void canController::init() {
  Serial.println("Initalizing CAN Controller");

  getBus().begin();

  getBus().setBaudRate(canConfig::CAN_BAUD_RATE);
  getBus().setMBFilter(REJECT_ALL);  // disable all mailboxes

  if (canConfig::canMonitorEnabled) {
    getBus().onReceive(sniffMessage);
  }

  if (canConfig::canMonitorEnabled) {
    getBus().enableFIFO();
    getBus().setFIFOFilter(ACCEPT_ALL);
    getBus().enableFIFOInterrupt();

    usedMailboxes += canConfig::FIFO_MAILBOX_COUNT;
  }

  // Preserve a TX mailbox always
  usedMailboxes++;

  tempMailbox = FLEXCAN_MAILBOX(usedMailboxes++);
  getBus().setMB(tempMailbox, RX);  // set mailbox 1 to receive default
  getBus().setMBFilter(tempMailbox, REJECT_ALL);
  getBus().enableMBInterrupt(tempMailbox);
  
  getBus().events();
}

void canController::poll() {
  getBus().events();
}