#include "kyma/validation.hpp"
#include "kyma/analyzer.hpp"

namespace kyma {
std::vector<Diagnostic> validate(const std::vector<StmtPtr> &program) {
  return Analyzer().analyze(program);
}
} // namespace kyma
