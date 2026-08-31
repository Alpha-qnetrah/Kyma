#pragma once

#include "kyma/source/source_span.hpp"
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kyma {

enum class DiagnosticSeverity { Error, Warning };

struct DiagnosticLabel {
  SourceSpan span;
  std::string message;
};

struct RuntimeCallFrame {
  std::string function;
  SourceSpan span;
};

struct Diagnostic {
  std::string message;
  SourceSpan location;
  bool warning{false};
  Diagnostic() = default;
  Diagnostic(std::string diagnosticMessage, SourceSpan diagnosticLocation, bool isWarning)
      : message(std::move(diagnosticMessage)), location(diagnosticLocation), warning(isWarning) {}
  Diagnostic(std::string diagnosticMessage, SourceSpan diagnosticLocation, bool isWarning,
             std::string diagnosticCode)
      : message(std::move(diagnosticMessage)), location(diagnosticLocation), warning(isWarning),
        code(std::move(diagnosticCode)) {}
  std::string code{"K0000"};
  std::vector<DiagnosticLabel> labels;
  std::vector<std::string> notes;
  std::string help;
  std::vector<RuntimeCallFrame> callFrames;

  [[nodiscard]] DiagnosticSeverity severity() const {
    return warning ? DiagnosticSeverity::Warning : DiagnosticSeverity::Error;
  }
};

class KymaError : public std::runtime_error {
public:
  explicit KymaError(const Diagnostic &value)
      : std::runtime_error(value.message), diagnostic(value) {}
  Diagnostic diagnostic;
};

} // namespace kyma
