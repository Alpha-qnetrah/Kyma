#pragma once

#include "kyma/syntax/module_nodes.hpp"

namespace kyma {

// SyntaxTree is the single owner presented at the parsing seam. The v0.2 implementation still
// uses stable shared node handles internally while parser recovery is introduced; callers never
// own individual nodes and can migrate independently when the arena representation lands.
struct SyntaxTree {
  ParsedModule module;
};

} // namespace kyma
