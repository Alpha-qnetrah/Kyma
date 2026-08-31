#pragma once

#include "kyna/semantics/type_model.hpp"
#include "kyna/syntax/statement_nodes.hpp"

namespace kyna {

struct VarDecl {
  bool mutableBinding;
  std::string name;
  TypeRef type;
  bool hasType{false};
  ExprPtr initializer;
  bool exported{false};
};
struct Param {
  std::string name;
  TypeRef type;
};
struct FunctionDecl {
  std::string name;
  std::vector<Param> params;
  TypeRef returnType;
  bool hasReturnType{false};
  StmtPtr body;
  std::vector<std::string> modifiers;
  bool exported{false};
};
struct FieldDecl {
  std::string name;
  TypeRef type;
  ExprPtr initializer;
  std::vector<std::string> modifiers;
};
struct ClassDecl {
  std::string name;
  std::string parent;
  std::vector<FieldDecl> fields;
  std::vector<FunctionDecl> methods;
  std::vector<std::string> modifiers;
  std::vector<std::string> interfaces;
  bool exported{false};
};
struct InterfaceDecl {
  std::string name;
  std::vector<FieldDecl> fields;
  std::vector<FunctionDecl> methods;
  bool exported{false};
};
struct ImportDecl {
  std::string path;
  std::string alias;
};

struct Stmt {
  using Node = std::variant<VarDecl, ExprStmt, BlockStmt, IfStmt, WhileStmt, LoopStmt, BreakStmt,
                            ContinueStmt, ReturnStmt, ThrowStmt, TryStmt, FunctionDecl, ClassDecl,
                            InterfaceDecl, ImportDecl, InvalidStmt>;
  Node node;
  SourceSpan location;
};

} // namespace kyna
