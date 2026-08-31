#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/lexing/token.hpp"
#include "kyna/source/source_file.hpp"
#include <vector>

namespace kyna {

struct LexResult {
  std::vector<Token> tokens;
  std::vector<Diagnostic> diagnostics;

  [[nodiscard]] bool ok() const;
};

LexResult tokenize(const SourceFile &source);

} // namespace kyna
