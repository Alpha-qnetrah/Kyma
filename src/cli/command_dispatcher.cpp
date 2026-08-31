#include "cli_commands.hpp"
#include "kyma/diagnostics/diagnostic_renderer.hpp"
#include <fstream>
#include <sstream>

namespace kyma::cli {

std::string readInput(const std::string &path, std::istream &standardInput, std::string &error) {
  std::ostringstream contents;
  if (path == "-") {
    contents << standardInput.rdbuf();
    return contents.str();
  }
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    error = "cannot open '" + path + "'";
    return {};
  }
  contents << file.rdbuf();
  return contents.str();
}

int renderResult(const LanguageResult &result, const Options &options, LanguageSession &session,
                 std::ostream &errors) {
  if (!result.diagnostics.empty()) {
    errors << (options.jsonDiagnostics
                   ? renderJsonDiagnostics(result.diagnostics, session.sourceManager())
                   : renderCompilerDiagnostics(result.diagnostics, session.sourceManager(),
                                               {options.color}))
           << '\n';
  }
  if (result.ok())
    return 0;
  for (const auto &diagnostic : result.diagnostics)
    if (diagnostic.code == "K4000" || diagnostic.code == "K4001")
      return 2;
  return 1;
}

int dispatch(const Options &options, std::istream &input, std::ostream &output,
             std::ostream &errors) {
  if (options.command == Command::Invalid) {
    errors << "kyma: " << options.error << "\nTry 'kyma --help'.\n";
    return 2;
  }
  if (options.command == Command::Help) {
    output << "Kyma 0.2.2 language tools\n\n"
              "Usage:\n"
              "  kyma run <file|-> [options]\n"
              "  kyma check <file|-> [options]\n"
              "  kyma repl\n"
              "  kyma tokens <file|-> [--format text|json]\n"
              "  kyma ast <file|-> [--format text|json]\n"
              "  kyma <file.ky>\n\n"
              "Options:\n"
              "  --module-path <dir>          Add a module search root (repeatable)\n"
              "  --diagnostic-format <kind>  text or json\n"
              "  --no-color                  Disable ANSI diagnostic colors\n";
    return 0;
  }
  if (options.command == Command::Version) {
    output << "Kyma 0.2.2\n";
    return 0;
  }
  if (options.command == Command::Repl)
    return runRepl(options, input, output, errors);
  LanguageSessionOptions sessionOptions;
  sessionOptions.modulePaths = options.modulePaths;
  LanguageSession session(std::move(sessionOptions));
  switch (options.command) {
  case Command::Run:
    return runSourceFile(options, session, input, output, errors);
  case Command::Check:
    return checkSourceFile(options, session, input, output, errors);
  case Command::Tokens:
    return dumpTokens(options, session, input, output, errors);
  case Command::Ast:
    return dumpSyntax(options, session, input, output, errors);
  default:
    return 2;
  }
}

} // namespace kyma::cli
