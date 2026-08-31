#include "cli_commands.hpp"
#include <iostream>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

int main(int argc, char **argv) {
  auto options = kyna::cli::parseArguments(argc, argv);
#if defined(_WIN32)
  options.richTerminal = options.color && _isatty(_fileno(stderr));
#else
  options.richTerminal = options.color && isatty(fileno(stderr));
#endif
  return kyna::cli::dispatch(options, std::cin, std::cout, std::cerr);
}
