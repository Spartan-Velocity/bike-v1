#include "bmsController.hpp"

#include "config.hpp"

void BMSController::init() {
    Serial.println("Initalizing BMS Controller");

    CANController::requestMessages(bmsConfig::MSGID_RX_BMS, onBMSMsg);
}

void BMSController::onBMSMsg(const Message& msg) {
    // TODO: read bms data to screen
}