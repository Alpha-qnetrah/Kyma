#pragma once
#include "kyma/ast.hpp"
#include <map>
#include <memory>
#include <vector>
namespace kyma {
class Analyzer {
public: std::vector<Diagnostic> analyze(const std::vector<StmtPtr>& program); void setInteractive(bool enabled) { interactive = enabled; }
private: std::vector<Diagnostic> errors; struct Scope { std::map<std::string, TypeRef> types; std::map<std::string, bool> mutableBindings; std::shared_ptr<Scope> parent; }; std::shared_ptr<Scope> scope; std::map<std::string, FunctionDecl> functions; std::map<std::string, ClassDecl> classes; bool interactive{false}; TypeRef currentReturn{"void",false,{}}; bool inFunction{false};
  void stmt(const StmtPtr&); void warning(const std::string&, SourceLocation); TypeRef expr(const ExprPtr&); TypeRef merge(const TypeRef&, const TypeRef&); bool compatible(const TypeRef&, const TypeRef&); bool defined(const std::string&) const; void error(const std::string&, SourceLocation); Scope* bindingScope(const std::string&) const; bool alwaysReturns(const StmtPtr&) const;
};
}
