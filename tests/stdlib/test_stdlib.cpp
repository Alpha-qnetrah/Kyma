#include "kyna/interpreter.hpp"
#include "kyna/parser.hpp"
#include "kyna/validation.hpp"
#include <cassert>

int main() {
  auto program = kyna::Parser(kyna::lex("let values = [1, 2]; push(values, 3); print(len(values)); "
                                        "writeFile(\"kyna-stdlib-test.txt\", \"ok\"); "
                                        "print(readFile(\"kyna-stdlib-test.txt\")); "
                                        "print(processRun(\"true\"));"))
                     .parse();
  assert(kyna::validate(program).empty());
  kyna::Interpreter interpreter;
  interpreter.execute(program);
}
