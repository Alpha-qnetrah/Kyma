#pragma once

#include "kyma/diagnostics/diagnostic.hpp"
#include "kyma/lexing/token.hpp"
#include "kyma/source/source_file.hpp"
#include <vector>

namespace kyma {

struct LexResult {
  std::vector<Token> tokens;
  std::vector<Diagnostic> diagnostics;

  [[nodiscard]] bool ok() const;
};

LexResult tokenize(const SourceFile &source);

} // namespace kyma
