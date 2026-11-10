#pragma once

#include <stdint.h>
#include <cstdint>

class motorController {
 public:
  static void init();
  static void poll();

 private:
  inline static volatile bool paramRecieved{};
  inline static volatile bool paramOk{};
  inline static volatile uint16_t expectedAddr{};
};