#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/mir/mir_program.hpp"

namespace kyna {

struct MirVerificationResult {
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return diagnostics.empty(); }
};

[[nodiscard]] MirVerificationResult verifyMir(const MirProgram &program);

} // namespace kyna
