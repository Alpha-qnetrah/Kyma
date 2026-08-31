#pragma once

#include "kyma/execution/tree_walk_interpreter.hpp"
#include "kyma/modules/module_loader.hpp"
#include "kyma/semantics/module_analyzer.hpp"
#include "kyma/source/source_manager.hpp"
#include <filesystem>
#include <string>

namespace kyma {

struct LanguageSessionOptions {
  std::vector<std::filesystem::path> modulePaths;
  RuntimeCapabilities capabilities{productionRuntimeCapabilities()};
};

struct LanguageResult {
  std::vector<Diagnostic> diagnostics;
  bool executed{false};
  [[nodiscard]] bool ok() const;
};

struct InspectionResult {
  std::string output;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const;
};

class LanguageSession {
public:
  explicit LanguageSession(LanguageSessionOptions options = {});
  LanguageResult check(const std::filesystem::path &entry);
  LanguageResult run(const std::filesystem::path &entry);
  LanguageResult checkSource(std::string name, std::string source);
  LanguageResult checkSourceAtPath(const std::filesystem::path &entry, std::string source);
  LanguageResult runSource(std::string name, std::string source, bool interactive = false);
  InspectionResult inspectTokens(std::string name, std::string source, bool json = false);
  InspectionResult inspectSyntax(std::string name, std::string source, bool json = false);
  SourceManager &sourceManager() { return sources; }

private:
  LanguageSessionOptions options;
  SourceManager sources;
  TreeWalkInterpreter executor;

  AnalysisResult compile(const std::filesystem::path &entry, std::vector<Diagnostic> &frontEnd);
};

} // namespace kyma
