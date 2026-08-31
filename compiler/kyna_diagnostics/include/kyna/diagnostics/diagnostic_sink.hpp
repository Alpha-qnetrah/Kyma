#pragma once

#include "kyna/diagnostics/diagnostic.hpp"
#include <utility>

namespace kyna {

class DiagnosticSink {
public:
  void report(Diagnostic diagnostic) { entries.push_back(std::move(diagnostic)); }
  [[nodiscard]] bool hasErrors() const;
  [[nodiscard]] const std::vector<Diagnostic> &diagnostics() const { return entries; }
  std::vector<Diagnostic> take() { return std::move(entries); }

private:
  std::vector<Diagnostic> entries;
};

} // namespace kyna
