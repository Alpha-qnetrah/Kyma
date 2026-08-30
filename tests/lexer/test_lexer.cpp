#include "kyma/lexer.hpp"
#include <cassert>

int main() {
  auto tokens = kyma::lex("// comment\nlet values = [1, 2];");
  assert(tokens[0].kind == kyma::TokenKind::Let);
  assert(tokens[3].kind == kyma::TokenKind::LeftBracket);
  assert(tokens.back().kind == kyma::TokenKind::End);
}
