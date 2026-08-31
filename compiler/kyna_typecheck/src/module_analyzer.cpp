#include "kyna/semantics/module_analyzer.hpp"
#include "best_practice_checker.hpp"
#include "kyna/semantics/program_analyzer.hpp"
#include <algorithm>

namespace kyna {
namespace {

std::map<std::string, TypeRef> exportedTypes(const ModuleRecord &module) {
  std::map<std::string, TypeRef> exports;
  for (const auto &statement : module.syntax.module.declarations) {
    std::visit(
        [&](const auto &declaration) {
          using T = std::decay_t<decltype(declaration)>;
          if constexpr (std::is_same_v<T, VarDecl>) {
            if (declaration.exported)
              exports[declaration.name] =
                  declaration.hasType ? declaration.type : TypeRef{"any", false, {}};
          } else if constexpr (std::is_same_v<T, FunctionDecl>) {
            if (declaration.exported)
              exports[declaration.name] =
                  declaration.hasReturnType ? declaration.returnType : TypeRef{"any", false, {}};
          } else if constexpr (std::is_same_v<T, ClassDecl>) {
            if (declaration.exported)
              exports[declaration.name] = TypeRef{"class:" + declaration.name, false, {}};
          } else if constexpr (std::is_same_v<T, InterfaceDecl>) {
            if (declaration.exported)
              exports[declaration.name] = TypeRef{declaration.name, false, {}};
          }
        },
        statement->node);
  }
  return exports;
}

} // namespace

bool AnalysisResult::ok() const {
  return program.has_value() &&
         std::none_of(diagnostics.begin(), diagnostics.end(),
                      [](const Diagnostic &diagnostic) { return !diagnostic.warning; });
}

AnalysisResult analyzeModuleGraph(ParsedModuleGraph graph) {
  std::vector<Diagnostic> diagnostics;
  for (const auto &path : graph.initializationOrder) {
    auto found = graph.modules.find(path);
    if (found == graph.modules.end())
      continue;
    std::map<std::string, TypeRef> imports;
    std::map<std::string, std::map<std::string, TypeRef>> moduleExports;
    for (const auto &dependency : found->second.dependencies) {
      imports[dependency.alias] = TypeRef{"module:" + dependency.alias, false, {}};
      if (const auto module = graph.modules.find(dependency.canonicalPath);
          module != graph.modules.end())
        moduleExports[dependency.alias] = exportedTypes(module->second);
    }
    Analyzer analyzer;
    analyzer.setExternalBindings(std::move(imports));
    analyzer.setModuleExports(std::move(moduleExports));
    auto moduleDiagnostics = analyzer.analyze(found->second.syntax.module.declarations);
    diagnostics.insert(diagnostics.end(), moduleDiagnostics.begin(), moduleDiagnostics.end());
    auto practiceDiagnostics = checkBestPractices(found->second.syntax.module.declarations);
    diagnostics.insert(diagnostics.end(), practiceDiagnostics.begin(), practiceDiagnostics.end());
  }
  const bool failed = std::any_of(diagnostics.begin(), diagnostics.end(),
                                  [](const Diagnostic &diagnostic) { return !diagnostic.warning; });
  if (failed)
    return {std::nullopt, std::move(diagnostics)};
  return {CheckedProgram{std::move(graph)}, std::move(diagnostics)};
}

} // namespace kyna
