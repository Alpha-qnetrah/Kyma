#include "kyma/diagnostics/diagnostic_sink.hpp"
#include <algorithm>

namespace kyma {
bool DiagnosticSink::hasErrors() const {
  return std::any_of(entries.begin(), entries.end(),
                     [](const Diagnostic &entry) { return !entry.warning; });
}
} // namespace kyma
