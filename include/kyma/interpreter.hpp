#pragma once
#include "kyma/runtime.hpp"
#include <iostream>
namespace kyma {
class Interpreter {
public:
  Interpreter();
  Value execute(const std::vector<StmtPtr> &program);
  Value evaluate(const ExprPtr &expression);
  void execute(const StmtPtr &statement);
  Value invoke(const FunctionPtr &, const std::vector<Value> &, ObjectPtr thisObject = nullptr);
  Heap &heap() { return objectHeap; }
  const Heap &heap() const { return objectHeap; }
  std::shared_ptr<Environment> currentEnvironment() const { return environment; }
  std::shared_ptr<Environment> globals() const { return global; }

private:
  Heap objectHeap;
  std::shared_ptr<Environment> global;
  std::shared_ptr<Environment> environment;
  int loopDepth{0};
  struct Flow {
    enum Kind { None, Return, Break, Continue };
    Kind kind{None};
    Value value;
    std::string label;
  };
  Flow flow;
  Value eval(const ExprPtr &);
  void exec(const StmtPtr &);
  void execBlock(const BlockStmt &, std::shared_ptr<Environment>);
  Value binary(TokenKind, const Value &, const Value &);
  Value call(const Call &, const ExprPtr &);
  Value getMember(const Member &);
  void setMember(const ExprPtr &, const std::string &, Value);
  void setIndex(const ExprPtr &, const ExprPtr &, Value);
};
} // namespace kyma
