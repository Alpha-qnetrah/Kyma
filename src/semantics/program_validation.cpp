#include "kyna/analyzer.hpp"
#include "kyna/validation.hpp"

namespace kyna {
std::vector<Diagnostic> validate(const std::vector<StmtPtr> &program) {
  return Analyzer().analyze(program);
}
} // namespace kyna
