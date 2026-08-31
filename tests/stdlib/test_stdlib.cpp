#include "kyna/execution/tree_walk_engine.hpp"
#include "kyna/parsing/recursive_descent_parser.hpp"
#include "kyna/semantics/program_validation.hpp"
#include "kyna/stdlib/standard_library_catalog.hpp"
#include <cassert>

int main() {
  auto program = kyna::Parser(kyna::lex("let values = [1, 2]; push(values, 3); print(len(values)); "
                                        "writeFile(\"kyna-stdlib-test.txt\", \"ok\"); "
                                        "print(readFile(\"kyna-stdlib-test.txt\")); "
                                        "print(processRun(\"true\"));"))
                     .parse();
  assert(kyna::validate(program).empty());
  kyna::Interpreter interpreter(kyna::productionRuntimeCapabilities(),
                                kyna::installStandardLibrary);
  interpreter.execute(program);
}
