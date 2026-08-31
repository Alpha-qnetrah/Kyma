#include "kyna/parser.hpp"
#include "kyna/validation.hpp"
#include <cassert>

int main() {
  auto valid = kyna::Parser(kyna::lex("intf Printable { name: str; } let value: int = 1;")).parse();
  assert(kyna::validate(valid).empty());

  auto invalid = kyna::Parser(kyna::lex("set value = 1; value = 2;")).parse();
  auto diagnostics = kyna::validate(invalid);
  assert(!diagnostics.empty());
  assert(!diagnostics.front().warning);
}
