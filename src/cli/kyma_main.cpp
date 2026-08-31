#include "cli_commands.hpp"
#include <iostream>

int main(int argc, char **argv) {
  return kyma::cli::dispatch(kyma::cli::parseArguments(argc, argv), std::cin, std::cout, std::cerr);
}
