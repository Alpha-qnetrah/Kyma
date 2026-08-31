#include "kyna/semantics/program_analyzer.hpp"
#include "kyna/execution/tree_walk_engine.hpp"
#include "kyna/lexing/legacy_lexer.hpp"
#include "kyna/parsing/recursive_descent_parser.hpp"
#include "kyna/stdlib/standard_library_catalog.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
using namespace kyna;
static void run(const std::string &s) {
  auto p = Parser(lex(s)).parse();
  auto e = Analyzer().analyze(p);
  assert(e.empty());
  Interpreter i(productionRuntimeCapabilities(), installStandardLibrary);
  i.execute(p);
}
int main() {
  auto ts = lex("let x: int = 2;");
  assert(ts[0].kind == TokenKind::Let);
  assert(ts[6].kind == TokenKind::Semicolon);
  run("let x: int = 2; x = 3;");
  run("func add(a: int, b: int): int { return a + b; } print(add(2, 3));");
  run("let total = 0; loop (let i = 0; i < 3; i = i + 1) { total = total + i; } print(total);");
  run("class Animal { public name: str; public init(name: str) { self.name = name; } public func "
      "speak(): str { return self.name; } } let a = new Animal(\"cat\"); print(a.speak());");
  run("class A { public value: int; public init(value: int) { self.value = value; } public func "
      "get(): int { return self.value; } } class B extends A { public override func get(): int { "
      "return super.get() + 1; } } let b = new B(4); print(b.get());");
  run("let dynamic: any; dynamic = 1; dynamic = \"text\"; let maybe: str? = null; print(dynamic);");
  run("let obj = { name: \"Kyna\", version: 1 }; obj.name = \"Language\"; print(obj.name); "
      "print(len(keys(obj)));");
  run("let values = [1, 2, 3]; values[1] = 8; push(values, 4); print(len(values)); "
      "print(values[1]); print(pop(values));");
  run("print(processRun(\"true\"));");
  run("try { error(\"expected failure\"); } catch (message) { log(message); } "
      "console.log(\"console works\"); logColor(\"green\", \"colored\");");
  auto exceptionProgram = Parser(lex(
      "let trace = \"\"; try { trace = trace + \"try:\"; throw \"boom\"; } "
      "catch (failure) { trace = trace + failure.code + \":\" + failure.message + \":\" + "
      "failure.cause; } finally { trace = trace + \":finally-1\"; trace = trace + "
      "\":finally-2\"; } func throughFinally(): str { try { return \"returned\"; } "
      "finally { trace = trace + \":return-cleanup\"; } } set returned = throughFinally();"))
                              .parse();
  assert(Analyzer().analyze(exceptionProgram).empty());
  Interpreter exceptionInterpreter(productionRuntimeCapabilities(), installStandardLibrary);
  exceptionInterpreter.execute(exceptionProgram);
  assert(exceptionInterpreter.currentEnvironment() == exceptionInterpreter.globals());
  assert(std::get<std::string>(exceptionInterpreter.globals()->get("trace").value.data) ==
         "try:KRT2301:boom:boom:finally-1:finally-2:return-cleanup");
  assert(std::get<std::string>(exceptionInterpreter.currentEnvironment()->get("returned").value.data) ==
         "returned");

  auto uncaughtFinally = Parser(lex(
      "let cleanup = \"\"; try { throw \"uncaught\"; } finally { "
      "cleanup = cleanup + \"first\"; cleanup = cleanup + \"-second\"; }"))
                             .parse();
  assert(Analyzer().analyze(uncaughtFinally).empty());
  Interpreter uncaughtInterpreter(productionRuntimeCapabilities(), installStandardLibrary);
  bool observedUncaught = false;
  try {
    uncaughtInterpreter.execute(uncaughtFinally);
  } catch (const RuntimeThrownError &error) {
    observedUncaught = error.value && error.value->code == "KRT2301";
  }
  assert(observedUncaught);
  assert(std::get<std::string>(uncaughtInterpreter.globals()->get("cleanup").value.data) ==
         "first-second");
  run("let n = 2; set text = if (n == 2) { \"yes\" } else { \"no\" }; print(text); print(match (n) "
      "{ 1 => \"one\"; 2 => \"two\"; _ => \"other\"; });");
  auto gcProgram =
      Parser(lex("class Node { public next: Node?; } func make(): void { let n = new Node(); "
                 "n.next = n; return; } loop (let i = 0; i < 300; i = i + 1) { make(); }"))
          .parse();
  assert(Analyzer().analyze(gcProgram).empty());
  Interpreter gcInterpreter(productionRuntimeCapabilities(), installStandardLibrary);
  auto baselineObjects = gcInterpreter.heap().live();
  gcInterpreter.heap().setThreshold(1);
  gcInterpreter.execute(gcProgram);
  assert(gcInterpreter.heap().collections() > 0);
  assert(gcInterpreter.heap().live() == baselineObjects);
  auto bad = Parser(lex("let x: int = \"bad\";")).parse();
  assert(!Analyzer().analyze(bad).empty());
  std::cout << "all Kyna tests passed\n";
}
