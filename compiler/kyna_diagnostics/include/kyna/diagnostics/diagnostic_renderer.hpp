#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/source/source_manager.hpp"
#include <string>
#include <vector>

namespace kyna {

struct DiagnosticRenderOptions {
  bool color{false};
};

std::string renderCompilerDiagnostics(const std::vector<Diagnostic> &diagnostics,
                                      const SourceManager &sources,
                                      DiagnosticRenderOptions options = {});
std::string renderJsonDiagnostics(const std::vector<Diagnostic> &diagnostics,
                                  const SourceManager &sources);

} // namespace kyna
