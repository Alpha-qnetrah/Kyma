#pragma once

#include "kyma/diagnostics/diagnostic.hpp"
#include "kyma/modules/module_graph.hpp"
#include "kyma/source/source_manager.hpp"
#include <filesystem>
#include <vector>

namespace kyma {

struct ModuleLoadOptions {
  std::vector<std::filesystem::path> modulePaths;
};

struct ModuleLoadResult {
  ParsedModuleGraph graph;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const;
};

ModuleLoadResult loadModuleGraph(SourceManager &sources, const std::filesystem::path &entry,
                                 const ModuleLoadOptions &options = {});
ModuleLoadResult loadModuleGraphWithEntrySource(SourceManager &sources,
                                                const std::filesystem::path &entry,
                                                std::string source,
                                                const ModuleLoadOptions &options = {});

} // namespace kyma
