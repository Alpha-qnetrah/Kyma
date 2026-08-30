#pragma once
#include "kyma/ast.hpp"
#include <vector>
namespace kyma {
class Parser {
public:
  explicit Parser(std::vector<Token> tokens); std::vector<StmtPtr> parse();
private:
  std::vector<Token> tokens; size_t current{0};
  const Token& peek() const; const Token& previous() const; bool check(TokenKind) const; bool match(TokenKind); const Token& consume(TokenKind, const std::string&);
  StmtPtr declaration(); StmtPtr statement(); StmtPtr block(); StmtPtr varDeclaration(); StmtPtr functionDeclaration(std::vector<std::string> modifiers); StmtPtr classDeclaration(std::vector<std::string> modifiers); StmtPtr interfaceDeclaration();
  ExprPtr expression(); ExprPtr assignment(); ExprPtr logicOr(); ExprPtr logicAnd(); ExprPtr equality(); ExprPtr comparison(); ExprPtr term(); ExprPtr factor(); ExprPtr unary(); ExprPtr call(); ExprPtr primary();
  TypeRef typeRef(); std::vector<std::string> modifiers(); std::vector<StmtPtr> parseStatementsUntil(TokenKind end, ExprPtr* tail = nullptr);
  ExprPtr make(Expr::Node node, SourceLocation l); StmtPtr make(Stmt::Node node, SourceLocation l);
};
}
