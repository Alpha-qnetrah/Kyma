#include "cli_commands.hpp"

namespace kyma::cli {
namespace {
Command commandNamed(const std::string &value) {
  if (value == "run")
    return Command::Run;
  if (value == "check")
    return Command::Check;
  if (value == "repl")
    return Command::Repl;
  if (value == "tokens")
    return Command::Tokens;
  if (value == "ast")
    return Command::Ast;
  return Command::Invalid;
}
} // namespace

Options parseArguments(int argc, char **argv) {
  Options options;
  if (argc == 1)
    return options;
  int index = 1;
  std::string first = argv[index++];
  if (first == "--help" || first == "-h") {
    options.command = Command::Help;
    return options;
  }
  if (first == "--version" || first == "-V") {
    options.command = Command::Version;
    return options;
  }
  if (first == "--repl") {
    options.command = Command::Repl;
  } else if (first == "--check") {
    options.command = Command::Check;
  } else {
    const auto explicitCommand = commandNamed(first);
    if (explicitCommand == Command::Invalid) {
      options.command = Command::Run;
      options.input = std::move(first);
    } else {
      options.command = explicitCommand;
    }
  }

  while (index < argc) {
    std::string argument = argv[index++];
    if (argument == "--module-path") {
      if (index >= argc) {
        options.command = Command::Invalid;
        options.error = "--module-path requires a directory";
        break;
      }
      options.modulePaths.emplace_back(argv[index++]);
    } else if (argument == "--diagnostic-format") {
      if (index >= argc) {
        options.command = Command::Invalid;
        options.error = "--diagnostic-format requires text or json";
        break;
      }
      const std::string format = argv[index++];
      if (format != "text" && format != "json") {
        options.command = Command::Invalid;
        options.error = "diagnostic format must be text or json";
        break;
      }
      options.jsonDiagnostics = format == "json";
    } else if (argument == "--format") {
      if (index >= argc) {
        options.command = Command::Invalid;
        options.error = "--format requires text or json";
        break;
      }
      const std::string format = argv[index++];
      if (format != "text" && format != "json") {
        options.command = Command::Invalid;
        options.error = "output format must be text or json";
        break;
      }
      options.jsonOutput = format == "json";
    } else if (argument == "--no-color") {
      options.color = false;
    } else if (argument == "--source-name") {
      if (index >= argc) {
        options.command = Command::Invalid;
        options.error = "--source-name requires a path";
        break;
      }
      options.sourceName = argv[index++];
    } else if (options.input.empty()) {
      options.input = std::move(argument);
    } else {
      options.command = Command::Invalid;
      options.error = "unexpected argument '" + argument + "'";
      break;
    }
  }
  if (options.command != Command::Repl && options.command != Command::Help &&
      options.command != Command::Version && options.command != Command::Invalid &&
      options.input.empty()) {
    options.command = Command::Invalid;
    options.error = "command requires a source file or '-'";
  }
  return options;
}

} // namespace kyma::cli
