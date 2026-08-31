#pragma once

#include "kyna/interpreter.hpp"
#include "kyna/semantics/checked_program.hpp"

namespace kyna {

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

} // namespace kyna
