#pragma once

#include "kyma/diagnostics/diagnostic.hpp"
#include "kyma/lexing/token.hpp"
#include "kyma/source/source_file.hpp"
#include "kyma/syntax/syntax_tree.hpp"
#include <vector>

namespace kyma {

struct ParseResult {
  SyntaxTree tree;
  std::vector<Diagnostic> diagnostics;
  bool incomplete{false};

  [[nodiscard]] bool ok() const;
};

ParseResult parseModule(const SourceFile &source, std::vector<Token> tokens);

} // namespace kyma
