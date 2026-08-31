#pragma once

#include "kyma/runtime.hpp"
#include <string>
#include <string_view>

namespace kyma {

class Interpreter;

Value parseJsonValue(std::string_view source, Interpreter &interpreter);
std::string stringifyJsonValue(const Value &value);

} // namespace kyma
