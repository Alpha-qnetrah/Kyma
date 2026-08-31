#include "kyma/parser.hpp"
#include "kyma/parsing/module_parser.hpp"
#include <algorithm>

namespace kyma {

bool ParseResult::ok() const {
  return std::none_of(diagnostics.begin(), diagnostics.end(),
                      [](const Diagnostic &diagnostic) { return !diagnostic.warning; });
}

ParseResult parseModule(const SourceFile &source, std::vector<Token> tokens) {
  return Parser(std::move(tokens)).parseRecovering(source);
}

} // namespace kyma
