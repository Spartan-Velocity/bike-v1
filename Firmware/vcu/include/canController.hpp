#pragma once

#include <FlexCAN_T4.h>
#include <cstdint>

#define U8(x) static_cast<uint8_t>(x)

#define CAN_DATA(b0, b1, b2, b3, b4, b5, b6, b7) \
  {U8(b0), U8(b1), U8(b2), U8(b3), U8(b4), U8(b5), U8(b6), U8(b7)}

namespace canController {

using Message = CAN_message_t;
using MessageData = uint8_t[8];

void requestMessages(uint32_t id, _MB_ptr handler, bool interupt);
void requestMessage(uint32_t id, _MB_ptr handler);

FLEXCAN_MAILBOX requestWriteMailbox();

// void write(const Message& msg);
void write(uint32_t id, const MessageData* data, uint8_t len);
void write(FLEXCAN_MAILBOX mailbox, uint32_t id, const MessageData* data, uint8_t len);

void init();
void poll();

}  // namespace canController