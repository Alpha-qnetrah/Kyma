#pragma once

#include "kyna/runtime.hpp"
#include <string>
#include <string_view>

namespace kyna {

class Interpreter;

Value parseJsonValue(std::string_view source, Interpreter &interpreter);
std::string stringifyJsonValue(const Value &value);

} // namespace kyna
