#include "kyma/words.hpp"

namespace kyma {
std::string tokenName(TokenKind kind) {
  switch (kind) {
  case TokenKind::End:
    return "end of file";
  case TokenKind::Identifier:
    return "identifier";
  case TokenKind::Int:
    return "integer";
  case TokenKind::Float:
    return "float";
  case TokenKind::String:
    return "string";
  case TokenKind::Char:
    return "character";
  case TokenKind::Semicolon:
    return "';'";
  case TokenKind::RightBrace:
    return "'}'";
  case TokenKind::LeftBrace:
    return "'{'";
  case TokenKind::RightParen:
    return "')'";
  default:
    return "token";
  }
}
} // namespace kyma
