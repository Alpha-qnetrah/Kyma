#include "cli_commands.hpp"

namespace kyna::cli {

int runSourceFile(const Options &options, LanguageSession &session, std::istream &input,
                  std::ostream &, std::ostream &errors) {
  if (options.input != "-")
    return renderResult(session.run(options.input), options, session, errors);
  std::string readError;
  auto source = readInput(options.input, input, readError);
  if (!readError.empty()) {
    errors << "kyna: " << readError << '\n';
    return 2;
  }
  return renderResult(session.runSource("<stdin>", std::move(source)), options, session, errors);
}

} // namespace kyna::cli
