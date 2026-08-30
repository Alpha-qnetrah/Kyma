#include "kyma/parser.hpp"
#include <cassert>

int main() {
  auto program =
      kyma::Parser(kyma::lex("class Box { public value: int; } let b = { value: 1 };")).parse();
  assert(program.size() == 2);
  assert(std::holds_alternative<kyma::ClassDecl>(program[0]->node));
  assert(std::holds_alternative<kyma::VarDecl>(program[1]->node));
}
