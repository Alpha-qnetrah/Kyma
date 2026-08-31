#pragma once
#include "kyna/diagnostics/diagnostic.hpp"
#include <string>

namespace kyna {
inline std::string formatDiagnostic(const Diagnostic &d, const std::string &file = "<source>") {
  return file + ":" + std::to_string(d.location.line) + ":" + std::to_string(d.location.column) +
         ": " + (d.warning ? "warning" : "error") + "[" + d.code + "]: " + d.message;
}
} // namespace kyna
