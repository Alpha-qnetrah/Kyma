#include "cli_commands.hpp"
#include "kyna/diagnostics/diagnostic_renderer.hpp"
#include "kyna/lexing/tokenizer.hpp"
#include "kyna/parsing/module_parser.hpp"
#include <memory>

namespace kyna::cli {

int runRepl(const Options &options, std::istream &input, std::ostream &output,
            std::ostream &errors) {
  auto session = std::make_unique<LanguageSession>(LanguageSessionOptions{options.modulePaths});
  std::string pending;
  std::string line;
  output << "Kyna 0.3.0 REPL (:help for commands, :quit to exit)\n>> ";
  while (std::getline(input, line)) {
    if (pending.empty() && (line == ":quit" || line == ":q"))
      break;
    if (pending.empty() && line == ":help") {
      output << ":quit  exit\n:reset clear declarations and values\n"
                ":tokens <code> inspect tokens\n:ast <code> inspect syntax\n>> ";
      continue;
    }
    if (pending.empty() && line == ":reset") {
      session = std::make_unique<LanguageSession>(LanguageSessionOptions{options.modulePaths});
      output << "session reset\n>> ";
      continue;
    }
    if (pending.empty() && line.starts_with(":tokens ")) {
      auto result = session->inspectTokens("<repl>", line.substr(8), options.jsonOutput);
      output << result.output;
      output << ">> ";
      continue;
    }
    if (pending.empty() && line.starts_with(":ast ")) {
      auto result = session->inspectSyntax("<repl>", line.substr(5), options.jsonOutput);
      output << result.output;
      output << ">> ";
      continue;
    }

    pending += line + '\n';
    SourceFile probe{UnknownSource, "<repl>", pending};
    auto lexed = tokenize(probe);
    auto parsed = parseModule(probe, std::move(lexed.tokens));
    if (parsed.incomplete) {
      output << ".. ";
      continue;
    }
    auto result = session->runSource("<repl>", pending, true);
    renderResult(result, options, *session, errors);
    pending.clear();
    output << ">> ";
  }
  if (!pending.empty())
    errors << "incomplete input at end of REPL\n";
  return 0;
}

} // namespace kyna::cli
