#pragma once

#include <cstdint>

namespace displayController {

void writeEnd();

void writeVal(const char* obj, int32_t value);

void writeTxt(const char* obj, const char* text);

void init();

} // namespace displayController