#pragma once
#include "kyma/lexer.hpp"
#include <string>

namespace kyma {
// The lexical keyword table is a separate module so adding syntax does not
// require editing the scanner implementation.
TokenKind keywordKind(const std::string &word);
} // namespace kyma
