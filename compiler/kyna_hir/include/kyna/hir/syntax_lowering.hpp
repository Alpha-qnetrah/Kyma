#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/hir/hir_program.hpp"
#include "kyna/syntax/syntax_tree.hpp"
#include <optional>

namespace kyna {

struct HirLoweringResult {
  std::optional<HirProgram> program;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return program.has_value() && diagnostics.empty(); }
};

[[nodiscard]] HirLoweringResult lowerSyntaxToHir(const std::string &moduleName,
                                                 const SyntaxTree &tree);

} // namespace kyna
