#include "kyna/lexer.hpp"
#include <cassert>

int main() {
  auto tokens = kyna::lex("// comment\nlet values = [1, 2];");
  assert(tokens[0].kind == kyna::TokenKind::Let);
  assert(tokens[3].kind == kyna::TokenKind::LeftBracket);
  assert(tokens.back().kind == kyna::TokenKind::End);
}
