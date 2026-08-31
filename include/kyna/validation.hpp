#pragma once
#include "kyna/ast.hpp"
#include "kyna/diagnostics.hpp"
#include <vector>

namespace kyna {
// Frontend validation seam. It owns name, mutability, and type validation and
// guarantees callers receive diagnostics before execution is attempted.
std::vector<Diagnostic> validate(const std::vector<StmtPtr> &program);
} // namespace kyna
