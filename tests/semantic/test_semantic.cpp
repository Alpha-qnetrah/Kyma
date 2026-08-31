#include "kyma/parser.hpp"
#include "kyma/validation.hpp"
#include <cassert>

int main() {
  auto valid = kyma::Parser(kyma::lex("intf Printable { name: str; } let value: int = 1;")).parse();
  assert(kyma::validate(valid).empty());

  auto invalid = kyma::Parser(kyma::lex("set value = 1; value = 2;")).parse();
  auto diagnostics = kyma::validate(invalid);
  assert(!diagnostics.empty());
  assert(!diagnostics.front().warning);
}
