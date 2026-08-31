#pragma once

#include "kyma/diagnostics/diagnostic.hpp"
#include "kyma/modules/module_graph.hpp"
#include "kyma/semantics/checked_program.hpp"
#include <optional>

namespace kyma {

struct AnalysisResult {
  std::optional<CheckedProgram> program;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const;
};

AnalysisResult analyzeModuleGraph(ParsedModuleGraph graph);

} // namespace kyma
