#pragma once

#include "kyma/syntax/declaration_nodes.hpp"
#include <filesystem>
#include <set>

namespace kyma {

struct ParsedModule {
  SourceId source{UnknownSource};
  std::filesystem::path path;
  std::vector<StmtPtr> declarations;
  std::set<std::string> exports;
};

} // namespace kyma
