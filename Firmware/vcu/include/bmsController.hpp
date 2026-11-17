#pragma once

#include <elapsedMillis.h>

#include "canController.hpp"

class BMSController {
  public:
    static void init();

  private:
    static void onBMSMsg(const Message& msg);
};