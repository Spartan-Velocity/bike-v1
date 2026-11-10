#pragma once

#include <FlexCAN_T4.h>
#include <cstdint>

#define U8(x) static_cast<uint8_t>(x)

#define CAN_DATA(b0, b1, b2, b3, b4, b5, b6, b7) \
  {U8(b0), U8(b1), U8(b2), U8(b3), U8(b4), U8(b5), U8(b6), U8(b7)}

using Message = CAN_message_t;
using MessageData = uint8_t[8];

class canController {
 public:
  static void requestMessages(uint32_t id, _MB_ptr handler, bool interupt);
  static void requestMessage(uint32_t id, _MB_ptr handler);

  static FLEXCAN_MAILBOX requestWriteMailbox();

  // void write(const Message& msg);
  static void write(uint32_t id, const MessageData* data, uint8_t len);
  static void write(FLEXCAN_MAILBOX mailbox, uint32_t id, const MessageData* data, uint8_t len);

  static void init();
  static void poll();

 private:
  inline static uint8_t usedMailboxes{};

  static void sniffMessage(const Message& msg);

  static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> getBus();
};