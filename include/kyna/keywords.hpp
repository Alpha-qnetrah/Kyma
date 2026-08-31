#pragma once
#include "kyna/lexing/token.hpp"
#include <string>

namespace kyna {
// The lexical keyword table is a separate module so adding syntax does not
// require editing the scanner implementation.
TokenKind keywordKind(const std::string &word);
} // namespace kyna
