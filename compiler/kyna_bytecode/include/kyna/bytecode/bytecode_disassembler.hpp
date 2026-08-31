#pragma once

#include "kyna/bytecode/bytecode_module.hpp"
#include <string>

namespace kyna {

[[nodiscard]] std::string disassembleBytecode(const BytecodeModule &module);

} // namespace kyna
