#pragma once

#include <FlexCAN_T4.h>
#include <cstdint>

namespace canController {

using Message = CAN_message_t;

void requestMessages(uint32_t id, _MB_ptr handler, bool interupt);
void write(const Message& msg);

void write(FLEXCAN_MAILBOX mailbox, uint32_t id, const uint8_t* data, uint8_t len);

void init();
void poll();

}  // namespace canController