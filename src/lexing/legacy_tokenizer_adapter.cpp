#include "kyma/lexer.hpp"
#include "kyma/lexing/tokenizer.hpp"

namespace kyma {
std::vector<Token> lex(const std::string &source) {
  auto result = tokenize(SourceFile{UnknownSource, {}, source});
  if (!result.diagnostics.empty())
    throw KymaError(result.diagnostics.front());
  return std::move(result.tokens);
}
} // namespace kyma
