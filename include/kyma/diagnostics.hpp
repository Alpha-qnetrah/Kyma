#pragma once
#include "kyma/diagnostics/diagnostic.hpp"
#include <string>

namespace kyma {
inline std::string formatDiagnostic(const Diagnostic &d, const std::string &file = "<source>") {
  return file + ":" + std::to_string(d.location.line) + ":" + std::to_string(d.location.column) +
         ": " + (d.warning ? "warning" : "error") + "[" + d.code + "]: " + d.message;
}
} // namespace kyma
