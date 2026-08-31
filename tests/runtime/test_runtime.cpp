#include "kyna/interpreter.hpp"
#include "kyna/parser.hpp"
#include "kyna/validation.hpp"
#include <cassert>

int main() {
  auto program =
      kyna::Parser(kyna::lex("class Node { public next: Node?; }"
                             "func make(): void { let n = new Node(); n.next = n; return; }"
                             "loop (let i = 0; i < 300; i = i + 1) { make(); }"))
          .parse();
  assert(kyna::validate(program).empty());
  kyna::Interpreter interpreter;
  auto baselineObjects = interpreter.heap().live();
  interpreter.heap().setThreshold(1);
  interpreter.execute(program);
  assert(interpreter.heap().collections() > 0);
  assert(interpreter.heap().live() == baselineObjects);
}
