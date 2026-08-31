#pragma once

#include "kyna/execution/tree_walk_engine.hpp"
#include "kyna/semantics/checked_program.hpp"

namespace kyna {

struct ExecutionResult {
  Value value;
  std::vector<Diagnostic> diagnostics;
  [[nodiscard]] bool ok() const { return diagnostics.empty(); }
};

class TreeWalkInterpreter {
public:
  explicit TreeWalkInterpreter(RuntimeCapabilities capabilities = productionRuntimeCapabilities(),
                               RuntimeInitializer initialize = {})
      : interpreter(std::move(capabilities), std::move(initialize)) {}
  ExecutionResult execute(const CheckedProgram &program);
  Interpreter &runtime() { return interpreter; }

private:
  Interpreter interpreter;
  std::map<std::filesystem::path, ModulePtr> initializedModules;
};

} // namespace kyna
