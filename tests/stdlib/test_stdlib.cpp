#include "kyma/interpreter.hpp"
#include "kyma/parser.hpp"
#include "kyma/validation.hpp"
#include <cassert>

int main() {
  auto program = kyma::Parser(kyma::lex("let values = [1, 2]; push(values, 3); print(len(values)); "
                                        "writeFile(\"kyma-stdlib-test.txt\", \"ok\"); "
                                        "print(readFile(\"kyma-stdlib-test.txt\")); "
                                        "print(processRun(\"true\"));"))
                     .parse();
  assert(kyma::validate(program).empty());
  kyma::Interpreter interpreter;
  interpreter.execute(program);
}
