#pragma once

#include "kyna/bytecode/bytecode_module.hpp"
#include "kyna/diagnostics/diagnostic.hpp"
#include <vector>

namespace kyna {

struct BytecodeValidationResult {
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return diagnostics.empty(); }
};

[[nodiscard]] BytecodeValidationResult validateBytecode(const BytecodeModule &module);

} // namespace kyna
