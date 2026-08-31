#include "kyna/execution/tree_walk_interpreter.hpp"
#include <algorithm>

namespace kyna {

ExecutionResult TreeWalkInterpreter::execute(const CheckedProgram &program) {
  try {
    Value last;
    for (const auto &path : program.modules.initializationOrder) {
      if (initializedModules.contains(path))
        continue;
      const auto found = program.modules.modules.find(path);
      if (found == program.modules.modules.end())
        continue;
      auto environment = interpreter.createModuleEnvironment();
      for (const auto &dependency : found->second.dependencies) {
        const auto imported = initializedModules.find(dependency.canonicalPath);
        if (imported == initializedModules.end()) {
          Diagnostic diagnostic{"module dependency was not initialized", dependency.location,
                                false};
          diagnostic.code = "K5001";
          return {{}, {std::move(diagnostic)}};
        }
        environment->define(dependency.alias, Value(imported->second), false);
      }
      last = interpreter.executeIn(found->second.syntax.module.declarations, environment);
      auto module = std::make_shared<ModuleNamespace>();
      module->environment = std::move(environment);
      module->exports = found->second.syntax.module.exports;
      module->displayName = path.filename().string();
      initializedModules.insert_or_assign(path, std::move(module));
    }
    return {std::move(last), {}};
  } catch (const KynaError &error) {
    auto diagnostic = error.diagnostic;
    if (diagnostic.code == "K0000")
      diagnostic.code = "K5000";
    return {{}, {std::move(diagnostic)}};
  } catch (const std::exception &error) {
    Diagnostic diagnostic{std::string("runtime failure: ") + error.what(), {}, false};
    diagnostic.code = "K5099";
    return {{}, {std::move(diagnostic)}};
  }
}

} // namespace kyna
