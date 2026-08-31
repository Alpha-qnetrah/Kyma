#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/modules/module_graph.hpp"
#include "kyna/semantics/checked_program.hpp"
#include <optional>

namespace kyna {

struct AnalysisResult {
  std::optional<CheckedProgram> program;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const;
};

AnalysisResult analyzeModuleGraph(ParsedModuleGraph graph);

} // namespace kyna
