#include "displayController.hpp"

#include "config.hpp"
#include "elapsedMillis.h"
#include "usb_serial.h"

void displayController::writeEnd() {
  displayConfig::NEXTION.write(0xFF);
  displayConfig::NEXTION.write(0xFF);
  displayConfig::NEXTION.write(0xFF);
}

void displayController::writeVal(const char* obj, int32_t value) {
  displayConfig::NEXTION.print(obj);
  displayConfig::NEXTION.print(".val=");
  displayConfig::NEXTION.print(value);

  writeEnd();
}

void displayController::writeTxt(const char* obj, const char* text) {
  displayConfig::NEXTION.print(obj);
  displayConfig::NEXTION.print(".txt=\"");
  displayConfig::NEXTION.print(text);
  displayConfig::NEXTION.print("\"");

  writeEnd();
}

void displayController::writeCmd(const char* cmd) {
  displayConfig::NEXTION.print(cmd);

  writeEnd();
}

void displayController::init() {
  displayConfig::NEXTION.begin(displayConfig::DISPLAY_BAUD);

  Serial.println("Connecting to Nextion...");

  auto timeout = elapsedMillis();

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