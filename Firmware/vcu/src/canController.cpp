#include "canController.hpp"

#include <FlexCAN_T4.h>

#include "config.hpp"

auto CANController::getBus() -> FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>& {
    static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> bus;
    return bus;
};

void CANController::sniffMessage(const Message& msg) {
    Serial.printf("%10lu %c %03lX [%u]", micros(), msg.flags.extended ? 'X' : 'S',
                  static_cast<uint32_t>(msg.id), msg.len);
    for (uint8_t i = 0; i < msg.len; i++) {
        Serial.printf(" %02X", msg.buf[i]);
    }
    Serial.println();
}

void CANController::requestMessages(uint32_t id, _MB_ptr handler) {
    auto mailbox = FLEXCAN_MAILBOX(usedMailboxes);

    getBus().setMB(mailbox, RX);
    getBus().setMBFilter(mailbox, id);
    getBus().onReceive(mailbox, handler);
    getBus().enableMBInterrupt(mailbox);

    usedMailboxes++;
}

void CANController::requestMessageTemp(uint32_t id, _MB_ptr handler) {
    getBus().setMBFilter(tempBox, id);
    getBus().onReceive(tempBox, handler);
}

auto CANController::requestWriteMailbox() -> FLEXCAN_MAILBOX {
    return FLEXCAN_MAILBOX(usedMailboxes++);
}

void CANController::write(uint32_t id, const MessageData* data) {
    CAN_message_t msg;
    msg.id = id;
    msg.len = 8;
    memcpy(msg.buf, data, 8);

    getBus().write(msg);
}

void CANController::write(FLEXCAN_MAILBOX mailbox, uint32_t id, const MessageData* data) {
    CAN_message_t msg;
    msg.id = id;
    msg.len = 8;
    memcpy(msg.buf, data, 8);

    getBus().write(mailbox, msg);
}

void CANController::init() {
    Serial.println("Initalizing CAN Controller");

    getBus().begin();
    getBus().setBaudRate(canConfig::CAN_BAUD_RATE);

    if (canConfig::canMonitorEnabled) {
        getBus().onReceive(sniffMessage);

        // getBus().enableFIFO();
        // getBus().setFIFOFilter(ACCEPT_ALL);
        // getBus().enableFIFOInterrupt();

        // usedMailboxes += canConfig::FIFO_MAILBOX_COUNT;
    }

    // Preserve a TX mailbox always
    auto transmitBox = FLEXCAN_MAILBOX(usedMailboxes++);
    getBus().setMB(transmitBox, TX);

    tempBox = FLEXCAN_MAILBOX(usedMailboxes++);
    getBus().setMB(tempBox, RX);
    getBus().setMBFilter(tempBox, REJECT_ALL);
    getBus().enableMBInterrupt(tempBox);

    getBus().events();

    Serial.println("Initalized CAN Controller");
}

void CANController::poll() {
    getBus().events();
}