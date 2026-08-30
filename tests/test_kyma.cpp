#include "kyma/kyma.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
using namespace kyma;
static void run(const std::string&s){auto p=Parser(lex(s)).parse();auto e=Analyzer().analyze(p);assert(e.empty());Interpreter i;i.execute(p);}
int main(){
  auto ts=lex("let x: int = 2;");assert(ts[0].kind==TokenKind::Let);assert(ts[6].kind==TokenKind::Semicolon);
  run("let x: int = 2; x = 3;");
  run("func add(a: int, b: int): int { return a + b; } print(add(2, 3));");
  run("let total = 0; loop (let i = 0; i < 3; i = i + 1) { total = total + i; } print(total);");
  run("class Animal { public name: str; public init(name: str) { self.name = name; } public func speak(): str { return self.name; } } let a = new Animal(\"cat\"); print(a.speak());");
  run("class A { public value: int; public init(value: int) { self.value = value; } public func get(): int { return self.value; } } class B extends A { public override func get(): int { return super.get() + 1; } } let b = new B(4); print(b.get());");
  run("let dynamic: any; dynamic = 1; dynamic = \"text\"; let maybe: str? = null; print(dynamic);");
  run("let obj = { name: \"Kyma\", version: 1 }; obj.name = \"Language\"; print(obj.name);");
  run("let n = 2; set text = if (n == 2) { \"yes\" } else { \"no\" }; print(text); print(match (n) { 1 => \"one\"; 2 => \"two\"; _ => \"other\"; });");
  auto gcProgram=Parser(lex("class Node { public next: Node?; } func make(): void { let n = new Node(); n.next = n; return; } loop (let i = 0; i < 300; i = i + 1) { make(); }" )).parse(); assert(Analyzer().analyze(gcProgram).empty()); Interpreter gcInterpreter; gcInterpreter.heap().setThreshold(1); gcInterpreter.execute(gcProgram); assert(gcInterpreter.heap().collections() > 0); assert(gcInterpreter.heap().live() == 0);
  auto bad=Parser(lex("let x: int = \"bad\";")).parse(); assert(!Analyzer().analyze(bad).empty());
  std::cout<<"all Kyma tests passed\n";
}
