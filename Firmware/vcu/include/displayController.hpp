#pragma once

#include <HardwareSerial.h>

class DisplayController {
  public:
    static void writeVal(const char* obj, int32_t value);

    static void writeTxt(const char* obj, const char* text);

    static void init();

  private:
    static void writeEnd();

    static void writeCmd(const char* cmd);
};