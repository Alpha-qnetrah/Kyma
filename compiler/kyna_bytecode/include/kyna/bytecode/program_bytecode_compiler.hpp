#pragma once

#include "kyna/bytecode/bytecode_module.hpp"
#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/mir/mir_program.hpp"
#include <optional>

namespace kyna {

struct BytecodeCompileResult {
  std::optional<BytecodeModule> module;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return module.has_value() && diagnostics.empty(); }
};

[[nodiscard]] BytecodeCompileResult compileMirToBytecode(const MirProgram &program);

} // namespace kyna
