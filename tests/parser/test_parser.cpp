#include "kyna/parser.hpp"
#include <cassert>

int main() {
  auto program =
      kyna::Parser(kyna::lex("class Box { public value: int; } let b = { value: 1 };")).parse();
  assert(program.size() == 2);
  assert(std::holds_alternative<kyna::ClassDecl>(program[0]->node));
  assert(std::holds_alternative<kyna::VarDecl>(program[1]->node));
}
