#include "kyma/analyzer.hpp"
#include "kyma/validation.hpp"

namespace kyma {
std::vector<Diagnostic> validate(const std::vector<StmtPtr> &program) {
  return Analyzer().analyze(program);
}
} // namespace kyma
