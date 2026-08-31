#pragma once
#include "kyma/execution/runtime_capabilities.hpp"
#include "kyma/runtime.hpp"
#include <iostream>
namespace kyma {
class Interpreter {
public:
  explicit Interpreter(RuntimeCapabilities capabilities = productionRuntimeCapabilities());
  Value execute(const std::vector<StmtPtr> &program);
  Value executeIn(const std::vector<StmtPtr> &program, std::shared_ptr<Environment> module);
  Value evaluate(const ExprPtr &expression);
  void execute(const StmtPtr &statement);
  Value invoke(const FunctionPtr &, const std::vector<Value> &, ObjectPtr thisObject = nullptr);
  Heap &heap() { return objectHeap; }
  const Heap &heap() const { return objectHeap; }
  std::shared_ptr<Environment> currentEnvironment() const { return environment; }
  std::shared_ptr<Environment> globals() const { return global; }
  std::vector<Environment *> rootEnvironments() const;
  RuntimeCapabilities runtimeCapabilities() const { return capabilities; }
  std::shared_ptr<Environment> createModuleEnvironment() const {
    return std::make_shared<Environment>(global);
  }

private:
  Heap objectHeap;
  std::shared_ptr<Environment> global;
  std::shared_ptr<Environment> environment;
  std::vector<std::shared_ptr<Environment>> moduleRoots;
  RuntimeCapabilities capabilities;
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
