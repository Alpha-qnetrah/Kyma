#pragma once

#include "kyma/diagnostics/diagnostic.hpp"
#include "kyma/syntax/declaration_nodes.hpp"
#include <vector>

namespace kyma {

std::vector<Diagnostic> checkBestPractices(const std::vector<StmtPtr> &declarations);

} // namespace kyma
