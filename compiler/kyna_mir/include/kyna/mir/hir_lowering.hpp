#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/hir/hir_program.hpp"
#include "kyna/mir/mir_program.hpp"
#include <optional>

namespace kyna {

struct MirLoweringResult {
  std::optional<MirProgram> program;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return program.has_value() && diagnostics.empty(); }
};

[[nodiscard]] MirLoweringResult lowerHirToMir(const HirProgram &program);

} // namespace kyna
