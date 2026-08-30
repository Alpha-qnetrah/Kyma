#pragma once
#include <stdexcept>
#include <string>
#include <vector>

namespace kyma {
struct SourceLocation { int line{1}; int column{1}; };
struct Diagnostic { std::string message; SourceLocation location; bool warning{false}; };
class KymaError : public std::runtime_error {
public:
  explicit KymaError(const Diagnostic& d) : std::runtime_error(d.message), diagnostic(d) {}
  Diagnostic diagnostic;
};
inline std::string formatDiagnostic(const Diagnostic& d, const std::string& file = "<source>") {
  return file + ":" + std::to_string(d.location.line) + ":" +
         std::to_string(d.location.column) + ": " + (d.warning ? "warning: " : "error: ") + d.message;
}
}
