#pragma once
#include "kyma/lexing/token.hpp"
#include <string>
#include <vector>

namespace kyma {
// v0.1 compatibility. New embedders should use tokenize(SourceFile).
std::vector<Token> lex(const std::string &source);
} // namespace kyma
