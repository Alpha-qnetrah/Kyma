#pragma once
#include "kyma/lexer.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace kyma {
struct Expr; struct Stmt;
using ExprPtr = std::shared_ptr<Expr>; using StmtPtr = std::shared_ptr<Stmt>;
struct TypeRef { std::string name{"void"}; bool nullable{false}; std::vector<TypeRef> unionTypes; std::string str() const; };
struct Literal { enum class Kind { Null, Bool, Int, Float, String, Char }; Kind kind; std::string value; };
struct Variable { std::string name; };
struct SelfExpr {}; struct SuperExpr {};
struct Unary { TokenKind op; ExprPtr right; };
struct Binary { ExprPtr left; TokenKind op; ExprPtr right; };
struct Assign { ExprPtr target; ExprPtr value; };
struct Call { ExprPtr callee; std::vector<ExprPtr> args; };
struct Member { ExprPtr object; std::string name; };
struct NewExpr { std::string className; std::vector<ExprPtr> args; };
struct ObjectField { std::string name; ExprPtr value; };
struct ObjectExpr { std::vector<ObjectField> fields; };
struct IfExpr { ExprPtr condition; StmtPtr thenBranch; StmtPtr elseBranch; };
struct MatchArm { ExprPtr pattern; ExprPtr value; bool wildcard{false}; };
struct MatchExpr { ExprPtr subject; std::vector<MatchArm> arms; };
struct Expr { using Node = std::variant<Literal, Variable, SelfExpr, SuperExpr, Unary, Binary, Assign, Call, Member, NewExpr, ObjectExpr, IfExpr, MatchExpr>; Node node; SourceLocation location; };

struct VarDecl { bool mutableBinding; std::string name; TypeRef type; bool hasType{false}; ExprPtr initializer; };
struct ExprStmt { ExprPtr expression; };
struct BlockStmt { std::vector<StmtPtr> statements; ExprPtr tail; };
struct IfStmt { ExprPtr condition; StmtPtr thenBranch; StmtPtr elseBranch; };
struct WhileStmt { ExprPtr condition; StmtPtr body; std::string label; };
struct LoopStmt { StmtPtr initializer; ExprPtr condition; ExprPtr increment; StmtPtr body; std::string label; };
struct BreakStmt { std::string label; };
struct ContinueStmt { std::string label; };
struct ReturnStmt { ExprPtr value; };
struct Param { std::string name; TypeRef type; };
struct FunctionDecl { std::string name; std::vector<Param> params; TypeRef returnType; bool hasReturnType{false}; StmtPtr body; std::vector<std::string> modifiers; };
struct FieldDecl { std::string name; TypeRef type; ExprPtr initializer; std::vector<std::string> modifiers; };
struct ClassDecl { std::string name; std::string parent; std::vector<FieldDecl> fields; std::vector<FunctionDecl> methods; std::vector<std::string> modifiers; };
struct InterfaceDecl { std::string name; std::vector<FieldDecl> fields; std::vector<FunctionDecl> methods; };
struct Stmt { using Node = std::variant<VarDecl, ExprStmt, BlockStmt, IfStmt, WhileStmt, LoopStmt, BreakStmt, ContinueStmt, ReturnStmt, FunctionDecl, ClassDecl, InterfaceDecl>; Node node; SourceLocation location; };
}
