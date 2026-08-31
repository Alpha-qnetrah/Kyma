#pragma once

#include "kyma/syntax/syntax_tree.hpp"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace kyma {

struct ModuleDependency {
  std::string alias;
  std::filesystem::path canonicalPath;
  SourceSpan location;
};

struct ModuleRecord {
  SyntaxTree syntax;
  std::vector<ModuleDependency> dependencies;
};

struct ParsedModuleGraph {
  std::filesystem::path entry;
  std::map<std::filesystem::path, ModuleRecord> modules;
  std::vector<std::filesystem::path> initializationOrder;
};

} // namespace kyma
