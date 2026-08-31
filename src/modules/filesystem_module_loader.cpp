#include "kyma/lexing/tokenizer.hpp"
#include "kyma/modules/module_loader.hpp"
#include "kyma/parsing/module_parser.hpp"
#include <algorithm>
#include <set>

namespace kyma {
namespace {

class GraphLoader {
public:
  GraphLoader(
      SourceManager &sourceManager, const ModuleLoadOptions &loadOptions,
      std::optional<std::pair<std::filesystem::path, std::string>> entryOverlay = std::nullopt)
      : sources(sourceManager), options(loadOptions), overlay(std::move(entryOverlay)) {
    if (overlay)
      overlay->first = canonical(overlay->first);
  }

  ModuleLoadResult load(const std::filesystem::path &entryPath) {
    auto entry = canonical(entryPath);
    result.graph.entry = entry;
    visit(entry, {});
    return std::move(result);
  }

private:
  SourceManager &sources;
  const ModuleLoadOptions &options;
  ModuleLoadResult result;
  std::map<std::filesystem::path, int> state;
  std::vector<std::filesystem::path> stack;
  std::optional<std::pair<std::filesystem::path, std::string>> overlay;

  static std::filesystem::path canonical(const std::filesystem::path &path) {
    std::error_code error;
    auto value = std::filesystem::weakly_canonical(path, error);
    return error ? std::filesystem::absolute(path).lexically_normal() : value;
  }

  std::filesystem::path resolve(const std::filesystem::path &importer,
                                const std::string &requested) const {
    const auto relative = canonical(importer.parent_path() / requested);
    if (std::filesystem::exists(relative))
      return relative;
    for (const auto &root : options.modulePaths) {
      const auto candidate = canonical(root / requested);
      if (std::filesystem::exists(candidate))
        return candidate;
    }
    return relative;
  }

  void report(std::string message, SourceSpan span, std::string code) {
    Diagnostic diagnostic{std::move(message), span, false};
    diagnostic.code = std::move(code);
    result.diagnostics.push_back(std::move(diagnostic));
  }

  void visit(const std::filesystem::path &path, SourceSpan importSpan) {
    const auto existing = state[path];
    if (existing == 2)
      return;
    if (existing == 1) {
      std::string chain;
      auto start = std::find(stack.begin(), stack.end(), path);
      for (auto item = start; item != stack.end(); ++item) {
        if (!chain.empty())
          chain += " -> ";
        chain += item->filename().string();
      }
      chain += " -> " + path.filename().string();
      report("module import cycle: " + chain, importSpan, "K4002");
      return;
    }

    std::string loadError;
    std::optional<SourceId> sourceId;
    if (overlay && path == overlay->first)
      sourceId = sources.add(path.string(), overlay->second);
    else
      sourceId = sources.load(path, loadError);
    if (!sourceId) {
      report(loadError, importSpan, "K4000");
      return;
    }
    const auto *source = sources.find(*sourceId);
    auto lexed = tokenize(*source);
    result.diagnostics.insert(result.diagnostics.end(), lexed.diagnostics.begin(),
                              lexed.diagnostics.end());
    auto parsed = parseModule(*source, std::move(lexed.tokens));
    result.diagnostics.insert(result.diagnostics.end(), parsed.diagnostics.begin(),
                              parsed.diagnostics.end());

    state[path] = 1;
    stack.push_back(path);
    ModuleRecord record{std::move(parsed.tree), {}};
    std::set<std::string> aliases;
    for (const auto &statement : record.syntax.module.declarations) {
      const auto *import = std::get_if<ImportDecl>(&statement->node);
      if (!import)
        continue;
      if (!aliases.insert(import->alias).second) {
        report("duplicate module alias '" + import->alias + "'", statement->location, "K4003");
        continue;
      }
      const auto dependency = resolve(path, import->path);
      if (!std::filesystem::exists(dependency)) {
        report("cannot resolve module '" + import->path + "'", statement->location, "K4001");
        continue;
      }
      record.dependencies.push_back({import->alias, dependency, statement->location});
      visit(dependency, statement->location);
    }
    stack.pop_back();
    state[path] = 2;
    result.graph.modules.insert_or_assign(path, std::move(record));
    result.graph.initializationOrder.push_back(path);
  }
};

} // namespace

bool ModuleLoadResult::ok() const {
  return std::none_of(diagnostics.begin(), diagnostics.end(),
                      [](const Diagnostic &diagnostic) { return !diagnostic.warning; });
}

ModuleLoadResult loadModuleGraph(SourceManager &sources, const std::filesystem::path &entry,
                                 const ModuleLoadOptions &options) {
  return GraphLoader(sources, options).load(entry);
}

ModuleLoadResult loadModuleGraphWithEntrySource(SourceManager &sources,
                                                const std::filesystem::path &entry,
                                                std::string source,
                                                const ModuleLoadOptions &options) {
  return GraphLoader(sources, options,
                     std::pair<std::filesystem::path, std::string>{entry, std::move(source)})
      .load(entry);
}

} // namespace kyma
