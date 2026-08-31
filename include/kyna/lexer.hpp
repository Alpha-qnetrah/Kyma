#pragma once
#include "kyna/lexing/token.hpp"
#include <string>
#include <vector>

namespace kyna {
// v0.1 compatibility. New embedders should use tokenize(SourceFile).
std::vector<Token> lex(const std::string &source);
} // namespace kyna
