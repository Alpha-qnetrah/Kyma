#pragma once

#include "kyma/syntax/expression_nodes.hpp"
#include <string>
#include <variant>
#include <vector>

namespace kyma {

struct ExprStmt {
  ExprPtr expression;
};
struct BlockStmt {
  std::vector<StmtPtr> statements;
  ExprPtr tail;
};
struct IfStmt {
  ExprPtr condition;
  StmtPtr thenBranch;
  StmtPtr elseBranch;
};
struct WhileStmt {
  ExprPtr condition;
  StmtPtr body;
  std::string label;
};
struct LoopStmt {
  StmtPtr initializer;
  ExprPtr condition;
  ExprPtr increment;
  StmtPtr body;
  std::string label;
};
struct BreakStmt {
  std::string label;
};
struct ContinueStmt {
  std::string label;
};
struct ReturnStmt {
  ExprPtr value;
};
struct TryStmt {
  StmtPtr tryBranch;
  std::string catchName;
  StmtPtr catchBranch;
};
struct InvalidStmt {};

} // namespace kyma
