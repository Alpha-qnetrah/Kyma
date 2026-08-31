#include "kyna/language/language_session.hpp"
#include "kyna/analyzer.hpp"
#include "kyna/lexing/tokenizer.hpp"
#include "kyna/parsing/module_parser.hpp"
#include <algorithm>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace kyna {
namespace {
bool hasErrors(const std::vector<Diagnostic> &diagnostics) {
  return std::any_of(diagnostics.begin(), diagnostics.end(),
                     [](const Diagnostic &diagnostic) { return !diagnostic.warning; });
}

std::string escapeJson(std::string_view value) {
  std::ostringstream output;
  for (const char character : value) {
    switch (character) {
    case '\\':
      output << "\\\\";
      break;
    case '"':
      output << "\\\"";
      break;
    case '\n':
      output << "\\n";
      break;
    case '\r':
      output << "\\r";
      break;
    case '\t':
      output << "\\t";
      break;
    default:
      output << character;
      break;
    }
  }
  return output.str();
}

std::string statementKind(const Stmt &statement) {
  return std::visit(
      [](const auto &node) -> std::string {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, ImportDecl>)
          return "import";
        if constexpr (std::is_same_v<T, VarDecl>)
          return node.mutableBinding ? "let" : "set";
        if constexpr (std::is_same_v<T, FunctionDecl>)
          return "function";
        if constexpr (std::is_same_v<T, ClassDecl>)
          return "class";
        if constexpr (std::is_same_v<T, InterfaceDecl>)
          return "interface";
        if constexpr (std::is_same_v<T, BlockStmt>)
          return "block";
        if constexpr (std::is_same_v<T, IfStmt>)
          return "if";
        if constexpr (std::is_same_v<T, WhileStmt>)
          return "while";
        if constexpr (std::is_same_v<T, LoopStmt>)
          return "loop";
        if constexpr (std::is_same_v<T, ReturnStmt>)
          return "return";
        if constexpr (std::is_same_v<T, TryStmt>)
          return "try";
        if constexpr (std::is_same_v<T, ExprStmt>)
          return "expression";
        return "statement";
      },
      statement.node);
}

std::string statementName(const Stmt &statement) {
  return std::visit(
      [](const auto &node) -> std::string {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, ImportDecl>)
          return node.alias;
        if constexpr (std::is_same_v<T, VarDecl> || std::is_same_v<T, FunctionDecl> ||
                      std::is_same_v<T, ClassDecl> || std::is_same_v<T, InterfaceDecl>)
          return node.name;
        return {};
      },
      statement.node);
}

bool statementExported(const Stmt &statement) {
  return std::visit(
      [](const auto &node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, VarDecl> || std::is_same_v<T, FunctionDecl> ||
                      std::is_same_v<T, ClassDecl> || std::is_same_v<T, InterfaceDecl>)
          return node.exported;
        return false;
      },
      statement.node);
}
} // namespace

bool LanguageResult::ok() const { return !hasErrors(diagnostics); }
bool InspectionResult::ok() const { return !hasErrors(diagnostics); }

LanguageSession::LanguageSession(LanguageSessionOptions sessionOptions)
    : options(std::move(sessionOptions)), executor(options.capabilities) {}

AnalysisResult LanguageSession::compile(const std::filesystem::path &entry,
                                        std::vector<Diagnostic> &frontEnd) {
  auto loaded = loadModuleGraph(sources, entry, ModuleLoadOptions{options.modulePaths});
  frontEnd = loaded.diagnostics;
  if (!loaded.ok())
    return {std::nullopt, {}};
  return analyzeModuleGraph(std::move(loaded.graph));
}

LanguageResult LanguageSession::check(const std::filesystem::path &entry) {
  std::vector<Diagnostic> diagnostics;
  auto analysis = compile(entry, diagnostics);
  diagnostics.insert(diagnostics.end(), analysis.diagnostics.begin(), analysis.diagnostics.end());
  return {std::move(diagnostics), false};
}

LanguageResult LanguageSession::run(const std::filesystem::path &entry) {
  std::vector<Diagnostic> diagnostics;
  auto analysis = compile(entry, diagnostics);
  diagnostics.insert(diagnostics.end(), analysis.diagnostics.begin(), analysis.diagnostics.end());
  if (!analysis.program || hasErrors(diagnostics))
    return {std::move(diagnostics), false};
  auto execution = executor.execute(*analysis.program);
  diagnostics.insert(diagnostics.end(), execution.diagnostics.begin(), execution.diagnostics.end());
  return {std::move(diagnostics), execution.ok()};
}

LanguageResult LanguageSession::checkSource(std::string name, std::string source) {
  const auto sourceId = sources.add(name, std::move(source));
  const auto &file = *sources.find(sourceId);
  auto lexed = tokenize(file);
  auto parsed = parseModule(file, std::move(lexed.tokens));
  std::vector<Diagnostic> diagnostics = std::move(lexed.diagnostics);
  diagnostics.insert(diagnostics.end(), parsed.diagnostics.begin(), parsed.diagnostics.end());
  if (hasErrors(diagnostics))
    return {std::move(diagnostics), false};
  ParsedModuleGraph graph;
  graph.entry = name;
  graph.initializationOrder.push_back(name);
  graph.modules.emplace(name, ModuleRecord{std::move(parsed.tree), {}});
  auto analysis = analyzeModuleGraph(std::move(graph));
  diagnostics.insert(diagnostics.end(), analysis.diagnostics.begin(), analysis.diagnostics.end());
  return {std::move(diagnostics), false};
}

LanguageResult LanguageSession::checkSourceAtPath(const std::filesystem::path &entry,
                                                  std::string source) {
  auto loaded = loadModuleGraphWithEntrySource(sources, entry, std::move(source),
                                               ModuleLoadOptions{options.modulePaths});
  auto diagnostics = std::move(loaded.diagnostics);
  if (!loaded.ok())
    return {std::move(diagnostics), false};
  auto analysis = analyzeModuleGraph(std::move(loaded.graph));
  diagnostics.insert(diagnostics.end(), analysis.diagnostics.begin(), analysis.diagnostics.end());
  return {std::move(diagnostics), false};
}

LanguageResult LanguageSession::runSource(std::string name, std::string source, bool interactive) {
  const auto sourceId = sources.add(name, std::move(source));
  const auto &file = *sources.find(sourceId);
  auto lexed = tokenize(file);
  auto parsed = parseModule(file, std::move(lexed.tokens));
  std::vector<Diagnostic> diagnostics = std::move(lexed.diagnostics);
  diagnostics.insert(diagnostics.end(), parsed.diagnostics.begin(), parsed.diagnostics.end());
  if (hasErrors(diagnostics))
    return {std::move(diagnostics), false};
  Analyzer analyzer;
  analyzer.setInteractive(interactive);
  auto semantic = analyzer.analyze(parsed.tree.module.declarations);
  diagnostics.insert(diagnostics.end(), semantic.begin(), semantic.end());
  if (hasErrors(diagnostics))
    return {std::move(diagnostics), false};
  try {
    executor.runtime().execute(parsed.tree.module.declarations);
    return {std::move(diagnostics), true};
  } catch (const KynaError &error) {
    diagnostics.push_back(error.diagnostic);
    return {std::move(diagnostics), false};
  }
}

InspectionResult LanguageSession::inspectTokens(std::string name, std::string source, bool json) {
  const auto sourceId = sources.add(name, std::move(source));
  auto lexed = tokenize(*sources.find(sourceId));
  std::ostringstream output;
  if (json)
    output << "{\"version\":1,\"tokens\":[";
  for (std::size_t index = 0; index < lexed.tokens.size(); ++index) {
    const auto &token = lexed.tokens[index];
    if (json) {
      if (index)
        output << ',';
      output << "{\"kind\":\"" << tokenName(token.kind) << "\",\"lexeme\":\""
             << escapeJson(token.lexeme) << "\",\"line\":" << token.location.line
             << ",\"column\":" << token.location.column << '}';
    } else {
      output << token.location.line << ':' << token.location.column << ' ' << tokenName(token.kind)
             << "  " << token.lexeme << '\n';
    }
  }
  if (json)
    output << "]}";
  return {output.str(), std::move(lexed.diagnostics)};
}

InspectionResult LanguageSession::inspectSyntax(std::string name, std::string source, bool json) {
  const auto sourceId = sources.add(name, std::move(source));
  auto lexed = tokenize(*sources.find(sourceId));
  auto parsed = parseModule(*sources.find(sourceId), std::move(lexed.tokens));
  std::vector<Diagnostic> diagnostics = std::move(lexed.diagnostics);
  diagnostics.insert(diagnostics.end(), parsed.diagnostics.begin(), parsed.diagnostics.end());
  std::ostringstream output;
  if (json)
    output << "{\"version\":1,\"module\":\"" << escapeJson(name) << "\",\"declarations\":[";
  for (std::size_t index = 0; index < parsed.tree.module.declarations.size(); ++index) {
    const auto &statement = *parsed.tree.module.declarations[index];
    const auto kind = statementKind(statement);
    const auto declarationName = statementName(statement);
    if (json) {
      if (index)
        output << ',';
      output << "{\"kind\":\"" << kind << "\",\"name\":\"" << escapeJson(declarationName)
             << "\",\"exported\":" << (statementExported(statement) ? "true" : "false")
             << ",\"range\":{\"start\":{\"line\":" << statement.location.line
             << ",\"column\":" << statement.location.column
             << "},\"end\":{\"line\":" << statement.location.endLine
             << ",\"column\":" << statement.location.endColumn << "}}}";
    } else {
      output << '(' << (statementExported(statement) ? "export " : "") << kind;
      if (!declarationName.empty())
        output << ' ' << declarationName;
      output << " @" << statement.location.line << ':' << statement.location.column << ")\n";
    }
  }
  if (json)
    output << "]}";
  return {output.str(), std::move(diagnostics)};
}

} // namespace kyna
