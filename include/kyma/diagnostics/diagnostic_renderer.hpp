#pragma once

#include "kyma/diagnostics/diagnostic.hpp"
#include "kyma/source/source_manager.hpp"
#include <string>
#include <vector>

namespace kyma {

struct DiagnosticRenderOptions {
  bool color{false};
};

std::string renderCompilerDiagnostics(const std::vector<Diagnostic> &diagnostics,
                                      const SourceManager &sources,
                                      DiagnosticRenderOptions options = {});
std::string renderJsonDiagnostics(const std::vector<Diagnostic> &diagnostics,
                                  const SourceManager &sources);

} // namespace kyma
