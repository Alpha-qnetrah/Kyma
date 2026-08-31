#pragma once

#include "kyma/interpreter.hpp"
#include "kyma/semantics/checked_program.hpp"

namespace kyma {

struct ExecutionResult {
  Value value;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return diagnostics.empty(); }
};

class TreeWalkInterpreter {
public:
  explicit TreeWalkInterpreter(RuntimeCapabilities capabilities = productionRuntimeCapabilities())
      : interpreter(std::move(capabilities)) {}
  ExecutionResult execute(const CheckedProgram &program);
  Interpreter &runtime() { return interpreter; }

private:
  Interpreter interpreter;
  std::map<std::filesystem::path, ModulePtr> initializedModules;
};

} // namespace kyma
