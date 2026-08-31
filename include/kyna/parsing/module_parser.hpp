#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/lexing/token.hpp"
#include "kyna/source/source_file.hpp"
#include "kyna/syntax/syntax_tree.hpp"
#include <vector>

namespace kyna {

struct ParseResult {
  SyntaxTree tree;
  std::vector<Diagnostic> diagnostics;
  bool incomplete{false};

  [[nodiscard]] bool ok() const;
};

ParseResult parseModule(const SourceFile &source, std::vector<Token> tokens);

} // namespace kyna
