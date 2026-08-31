#include "kyma/interpreter.hpp"
#include "kyma/parser.hpp"
#include "kyma/validation.hpp"
#include <cassert>

int main() {
  auto program =
      kyma::Parser(kyma::lex("class Node { public next: Node?; }"
                             "func make(): void { let n = new Node(); n.next = n; return; }"
                             "loop (let i = 0; i < 300; i = i + 1) { make(); }"))
          .parse();
  assert(kyma::validate(program).empty());
  kyma::Interpreter interpreter;
  auto baselineObjects = interpreter.heap().live();
  interpreter.heap().setThreshold(1);
  interpreter.execute(program);
  assert(interpreter.heap().collections() > 0);
  assert(interpreter.heap().live() == baselineObjects);
}
