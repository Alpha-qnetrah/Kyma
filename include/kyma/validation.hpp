#pragma once
#include "kyma/ast.hpp"
#include "kyma/diagnostics.hpp"
#include <vector>

namespace kyma {
// Frontend validation seam. It owns name, mutability, and type validation and
// guarantees callers receive diagnostics before execution is attempted.
std::vector<Diagnostic> validate(const std::vector<StmtPtr> &program);
} // namespace kyma
