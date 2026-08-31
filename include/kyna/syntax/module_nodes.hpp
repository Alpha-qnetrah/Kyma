#pragma once

#include "kyna/syntax/declaration_nodes.hpp"
#include <filesystem>
#include <set>

namespace kyna {

struct ParsedModule {
  SourceId source{UnknownSource};
  std::filesystem::path path;
  std::vector<StmtPtr> declarations;
  std::set<std::string> exports;
};

} // namespace kyna
