#pragma once

#include "kyna/source/source_span.hpp"
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kyna {

enum class DiagnosticSeverity { Error, Warning };

struct DiagnosticLabel {
  SourceSpan span;
  std::string message;
};

struct RuntimeCallFrame {
  std::string function;
  SourceSpan span;
};

struct DiagnosticCause {
  std::string domain;
  std::string code;
  std::string message;
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
  std::string category{"language"};
  std::vector<DiagnosticLabel> labels;
  std::vector<std::string> notes;
  std::string help;
  std::vector<DiagnosticCause> causes;
  std::vector<RuntimeCallFrame> callFrames;

  [[nodiscard]] DiagnosticSeverity severity() const {
    return warning ? DiagnosticSeverity::Warning : DiagnosticSeverity::Error;
  }
};

class KynaError : public std::runtime_error {
public:
  explicit KynaError(const Diagnostic &value)
      : std::runtime_error(value.message), diagnostic(value) {}
  Diagnostic diagnostic;
};

} // namespace kyna
