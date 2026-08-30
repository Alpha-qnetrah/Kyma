#include "kyma/analyzer.hpp"

namespace kyma {
namespace {
TypeRef t(const std::string &name) { return TypeRef{name, false, {}}; }
} // namespace
TypeRef Analyzer::expr(const ExprPtr &e) {
  return std::visit(
      [this, &e](const auto &n) -> TypeRef {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, Literal>) {
          switch (n.kind) {
          case Literal::Kind::Null:
            return t("null");
          case Literal::Kind::Bool:
            return t("bool");
          case Literal::Kind::Int:
            return t("int");
          case Literal::Kind::Float:
            return t("float");
          case Literal::Kind::String:
            return t("str");
          case Literal::Kind::Char:
            return t("char");
          }
        } else if constexpr (std::is_same_v<T, Variable>) {
          if (!defined(n.name) && !interactive)
            error("undefined name '" + n.name + "'", e->location);
          if (auto bs = bindingScope(n.name))
            return bs->types[n.name];
          if (functions.contains(n.name))
            return t("func");
          if (classes.contains(n.name))
            return t("class");
          return t("any");
        } else if constexpr (std::is_same_v<T, SelfExpr>)
          return t("object");
        else if constexpr (std::is_same_v<T, SuperExpr>)
          return t("object");
        else if constexpr (std::is_same_v<T, Unary>) {
          auto x = expr(n.right);
          if (n.op == TokenKind::Bang)
            return t("bool");
          if (x.name != "int" && x.name != "float" && x.name != "num" && x.name != "any")
            error("unary '-' requires a numeric operand", e->location);
          return x;
        } else if constexpr (std::is_same_v<T, Binary>) {
          auto a = expr(n.left), b = expr(n.right);
          if (n.op == TokenKind::EqualEqual || n.op == TokenKind::BangEqual ||
              n.op == TokenKind::AndAnd || n.op == TokenKind::OrOr || n.op == TokenKind::Less ||
              n.op == TokenKind::LessEqual || n.op == TokenKind::Greater ||
              n.op == TokenKind::GreaterEqual)
            return t("bool");
          if (n.op == TokenKind::Plus && (a.name == "str" || b.name == "str"))
            return t("str");
          if (a.name == "any" || b.name == "any")
            return t("any");
          if ((a.name == "int" || a.name == "float" || a.name == "num" || a.name == "any") &&
              (b.name == "int" || b.name == "float" || b.name == "num" || b.name == "any"))
            return (a.name == "float" || b.name == "float")
                       ? t("float")
                       : (a.name == "int" && b.name == "int" ? t("int") : t("num"));
          error("operator requires compatible operands", e->location);
          return t("any");
        } else if constexpr (std::is_same_v<T, Assign>) {
          auto a = expr(n.target), b = expr(n.value);
          if (auto v = std::get_if<Variable>(&n.target->node)) {
            if (auto bs = bindingScope(v->name)) {
              if (!bs->mutableBindings[v->name])
                error("cannot assign to immutable binding '" + v->name + "'", e->location);
              if (!compatible(bs->types[v->name], b))
                error("cannot assign " + b.str() + " to " + bs->types[v->name].str(), e->location);
            }
          } else if (!std::holds_alternative<Member>(n.target->node) &&
                     !std::holds_alternative<Index>(n.target->node))
            error("invalid assignment target", e->location);
          return a;
        } else if constexpr (std::is_same_v<T, Call>) {
          auto c = expr(n.callee);
          for (auto &a : n.args)
            expr(a);
          if (auto v = std::get_if<Variable>(&n.callee->node); v && functions.contains(v->name)) {
            auto &f = functions[v->name];
            if (f.params.size() != n.args.size())
              error("wrong number of arguments to '" + v->name + "'", e->location);
            for (size_t i = 0; i < std::min(f.params.size(), n.args.size()); ++i) {
              auto at = expr(n.args[i]);
              if (!compatible(f.params[i].type, at))
                error("argument " + std::to_string(i + 1) + " to '" + v->name + "' has type " +
                          at.str() + ", expected " + f.params[i].type.str(),
                      e->location);
            }
            return f.hasReturnType ? f.returnType : t("any");
          }
          return t("any");
        } else if constexpr (std::is_same_v<T, Member>) {
          expr(n.object);
          return t("any");
        } else if constexpr (std::is_same_v<T, Index>) {
          auto object = expr(n.object);
          auto index = expr(n.index);
          if (index.name != "int" && index.name != "any")
            error("array index must be int", e->location);
          if (object.name != "array" && object.name != "any")
            error("indexing requires an array", e->location);
          return t("any");
        } else if constexpr (std::is_same_v<T, ArrayExpr>) {
          for (auto &element : n.elements)
            expr(element);
          return t("array");
        } else if constexpr (std::is_same_v<T, NewExpr>) {
          for (auto &a : n.args)
            expr(a);
          if (!classes.contains(n.className))
            error("unknown class '" + n.className + "'", e->location);
          return t(n.className);
        } else if constexpr (std::is_same_v<T, ObjectExpr>) {
          for (auto &f : n.fields)
            expr(f.value);
          return t("object");
        } else if constexpr (std::is_same_v<T, IfExpr>) {
          auto c = expr(n.condition);
          if (c.name != "bool" && c.name != "any")
            error("if condition must be bool", e->location);
          auto a = std::get<BlockStmt>(n.thenBranch->node).tail
                       ? expr(std::get<BlockStmt>(n.thenBranch->node).tail)
                       : t("void");
          auto b = std::get<BlockStmt>(n.elseBranch->node).tail
                       ? expr(std::get<BlockStmt>(n.elseBranch->node).tail)
                       : t("void");
          return merge(a, b);
        } else if constexpr (std::is_same_v<T, MatchExpr>) {
          expr(n.subject);
          TypeRef r = t("void");
          bool first = true;
          for (auto &a : n.arms) {
            if (!a.wildcard)
              expr(a.pattern);
            auto arm = expr(a.value);
            if (first) {
              r = arm;
              first = false;
            } else
              r = merge(r, arm);
          }
          return r;
        } else
          return t("any");
      },
      e->node);
}
} // namespace kyma
