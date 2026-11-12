#include "displayController.hpp"

#include <elapsedMillis.h>
#include <usb_serial.h>

#include "config.hpp"

void DisplayController::writeEnd() {
    displayConfig::NEXTION.write(0xFF);
    displayConfig::NEXTION.write(0xFF);
    displayConfig::NEXTION.write(0xFF);
}

void DisplayController::writeVal(const char* obj, int32_t value) {
    displayConfig::NEXTION.print(obj);
    displayConfig::NEXTION.print(".val=");
    displayConfig::NEXTION.print(value);

    writeEnd();
}

void DisplayController::writeTxt(const char* obj, const char* text) {
    displayConfig::NEXTION.print(obj);
    displayConfig::NEXTION.print(".txt=\"");
    displayConfig::NEXTION.print(text);
    displayConfig::NEXTION.print("\"");

    writeEnd();
}

void DisplayController::writeCmd(const char* cmd) {
    displayConfig::NEXTION.print(cmd);

    writeEnd();
}

void DisplayController::init() {
    Serial.println("Initalizing Display Controller");

    displayConfig::NEXTION.begin(displayConfig::DISPLAY_BAUD);

    elapsedMillis timeout{};

    while (displayConfig::NEXTION.available() == 0) {
        if (timeout > displayConfig::DISPLAY_TIMEOUT) {
            Serial.println("Nextion timeout");
            return;
        }
        writeCmd("connect");
    }

    writeCmd("blkcmd=3");

    while (displayConfig::NEXTION.available() != 0) {
        displayConfig::NEXTION.read();
    }

    Serial.println("Nextion connected");
}