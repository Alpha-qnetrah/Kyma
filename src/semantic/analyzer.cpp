#include "kyma/analyzer.hpp"
#include "kyma/behavior.hpp"
#include <algorithm>
#include <map>

namespace kyma {
namespace {
TypeRef t(const std::string &n) { return TypeRef{n, false, {}}; }
} // namespace
void Analyzer::error(const std::string &m, SourceLocation l) { errors.push_back({m, l, false}); }
void Analyzer::warning(const std::string &m, SourceLocation l) { errors.push_back({m, l, true}); }
Analyzer::Scope *Analyzer::bindingScope(const std::string &n) const {
  for (auto s = scope; s; s = s->parent)
    if (s->types.contains(n))
      return s.get();
  return nullptr;
}
bool Analyzer::defined(const std::string &n) const {
  return bindingScope(n) != nullptr || functions.contains(n) || classes.contains(n) ||
         n == "print" || n == "typeOf" || n == "collectGarbage" || n == "gcStats" || n == "len" ||
         n == "push" || n == "pop" || n == "keys" || n == "readFile" || n == "writeFile" ||
         n == "processRun" || n == "processEnv" || n == "sleep" || n == "httpGet" || n == "fetch" ||
         n == "build" || n == "wait" || n == "log" || n == "logColor" || n == "console" ||
         n == "error";
}
bool Analyzer::compatible(const TypeRef &e, const TypeRef &a) {
  if (e.name == "any" || a.name == "any")
    return true;
  if (a.name == "null")
    return e.nullable || e.name == "null" ||
           std::any_of(e.unionTypes.begin(), e.unionTypes.end(),
                       [](const auto &x) { return x.name == "null"; });
  if (e.name == "num" && (a.name == "int" || a.name == "float"))
    return true;
  if (e.name == a.name && (!a.nullable || e.nullable || e.name == "null"))
    return true;
  for (auto &u : e.unionTypes)
    if (compatible(u, a))
      return true;
  return false;
}
TypeRef Analyzer::merge(const TypeRef &a, const TypeRef &b) {
  if (compatible(a, b))
    return a;
  if (compatible(b, a))
    return b;
  TypeRef r{"union", false, {a, b}};
  return r;
}
std::vector<Diagnostic> Analyzer::analyze(const std::vector<StmtPtr> &p) {
  errors.clear();
  scope = std::make_shared<Scope>();
  functions.clear();
  classes.clear();
  interfaces.clear();
  for (auto &s : p) {
    if (auto f = std::get_if<FunctionDecl>(&s->node))
      functions[f->name] = *f;
    if (auto c = std::get_if<ClassDecl>(&s->node))
      classes[c->name] = *c;
    if (auto i = std::get_if<InterfaceDecl>(&s->node)) {
      if (!interfaces.declareInterface(*i))
        error("interface '" + i->name + "' is already declared", s->location);
    }
  }
  for (auto &s : p)
    stmt(s);
  return errors;
}
void Analyzer::stmt(const StmtPtr &s) {
  std::visit(
      [this](const auto &n) {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, VarDecl>) {
          for (auto p = scope->parent; p; p = p->parent)
            if (p->types.contains(n.name)) {
              warning("binding '" + n.name + "' shadows an outer binding", SourceLocation{});
              break;
            }
          if (n.initializer) {
            auto a = expr(n.initializer);
            if (n.hasType && !compatible(n.type, a))
              error("initializer of '" + n.name + "' has type " + a.str() + ", expected " +
                        n.type.str(),
                    n.initializer->location);
            scope->types[n.name] = n.hasType ? n.type : a;
            scope->mutableBindings[n.name] = n.mutableBinding;
          } else {
            if (n.type.name != "any")
              error("binding '" + n.name + "' needs an initializer (or explicit any)", {1, 1});
            scope->types[n.name] = n.type;
            scope->mutableBindings[n.name] = n.mutableBinding;
          }
        } else if constexpr (std::is_same_v<T, ExprStmt>)
          expr(n.expression);
        else if constexpr (std::is_same_v<T, BlockStmt>) {
          auto old = scope;
          scope = std::make_shared<Scope>();
          scope->parent = old;
          for (auto &x : n.statements)
            stmt(x);
          if (n.tail)
            expr(n.tail);
          scope = old;
        } else if constexpr (std::is_same_v<T, IfStmt>) {
          auto c = expr(n.condition);
          if (c.name != "bool" && c.name != "any")
            error("if condition must be bool", n.condition->location);
          stmt(n.thenBranch);
          if (n.elseBranch)
            stmt(n.elseBranch);
        } else if constexpr (std::is_same_v<T, WhileStmt>) {
          expr(n.condition);
          stmt(n.body);
        } else if constexpr (std::is_same_v<T, LoopStmt>) {
          if (n.initializer)
            stmt(n.initializer);
          if (n.condition)
            expr(n.condition);
          if (n.increment)
            expr(n.increment);
          stmt(n.body);
        } else if constexpr (std::is_same_v<T, TryStmt>) {
          stmt(n.tryBranch);
          auto old = scope;
          scope = std::make_shared<Scope>();
          scope->parent = old;
          scope->types[n.catchName] = t("str");
          scope->mutableBindings[n.catchName] = false;
          stmt(n.catchBranch);
          scope = old;
        } else if constexpr (std::is_same_v<T, BreakStmt> || std::is_same_v<T, ContinueStmt>) {
        } else if constexpr (std::is_same_v<T, ReturnStmt>) {
          if (!inFunction)
            error("return must be inside a function",
                  n.value ? n.value->location : SourceLocation{});
          TypeRef actual = n.value ? expr(n.value) : t("void");
          if (inFunction && !compatible(currentReturn, actual))
            error("return type " + actual.str() + " does not satisfy " + currentReturn.str(),
                  n.value ? n.value->location : SourceLocation{});
        } else if constexpr (std::is_same_v<T, FunctionDecl>) {
          auto old = scope;
          auto oldReturn = currentReturn;
          bool oldIn = inFunction;
          scope = std::make_shared<Scope>();
          scope->parent = old;
          for (auto &p : n.params) {
            scope->types[p.name] = p.type;
            scope->mutableBindings[p.name] = false;
          }
          currentReturn = n.hasReturnType ? n.returnType : t("any");
          inFunction = true;
          stmt(n.body);
          if (n.hasReturnType && n.returnType.name != "void" && !alwaysReturns(n.body))
            error("function '" + n.name + "' can reach its end without returning " +
                      n.returnType.str(),
                  SourceLocation{});
          scope = old;
          currentReturn = oldReturn;
          inFunction = oldIn;
        } else if constexpr (std::is_same_v<T, ClassDecl>) {
          if (!n.parent.empty() && classes.contains(n.parent)) {
            auto &base = classes[n.parent];
            if (hasModifier(base.modifiers, "final"))
              error("cannot extend final class '" + n.parent + "'", SourceLocation{});
            for (auto &m : n.methods) {
              auto inherited = std::find_if(base.methods.begin(), base.methods.end(),
                                            [&](const auto &x) { return x.name == m.name; });
              if (hasModifier(m.modifiers, "override") && inherited == base.methods.end())
                error("method '" + m.name + "' is marked override but no inherited method exists",
                      SourceLocation{});
              if (inherited != base.methods.end() && !hasModifier(m.modifiers, "override") &&
                  m.name != "init")
                error("overriding method '" + m.name + "' requires the override modifier",
                      SourceLocation{});
              if (inherited != base.methods.end() && hasModifier(inherited->modifiers, "final"))
                error("cannot override final method '" + m.name + "'", SourceLocation{});
              if (inherited != base.methods.end() && m.hasReturnType && inherited->hasReturnType &&
                  !compatible(inherited->returnType, m.returnType))
                error("override return type for '" + m.name + "' is incompatible",
                      SourceLocation{});
            }
          }
          for (auto &m : n.methods) {
            auto old = scope;
            auto oldReturn = currentReturn;
            bool oldIn = inFunction;
            scope = std::make_shared<Scope>();
            scope->parent = old;
            scope->types["self"] = t(n.name);
            scope->mutableBindings["self"] = false;
            for (auto &f : n.fields) {
              scope->types[f.name] = f.type;
              scope->mutableBindings[f.name] = true;
            }
            for (auto &p : m.params) {
              scope->types[p.name] = p.type;
              scope->mutableBindings[p.name] = false;
            }
            currentReturn = m.hasReturnType ? m.returnType : t("any");
            inFunction = true;
            stmt(m.body);
            if (m.hasReturnType && m.returnType.name != "void" && !alwaysReturns(m.body))
              error("method '" + m.name + "' can reach its end without returning " +
                        m.returnType.str(),
                    SourceLocation{});
            scope = old;
            currentReturn = oldReturn;
            inFunction = oldIn;
          }
          if (!n.parent.empty() && !classes.contains(n.parent))
            error("unknown parent class '" + n.parent + "'", {1, 1});
        } else if constexpr (std::is_same_v<T, InterfaceDecl>) {
        }
      },
      s->node);
}
bool Analyzer::alwaysReturns(const StmtPtr &s) const {
  if (!s)
    return false;
  if (std::holds_alternative<ReturnStmt>(s->node))
    return true;
  if (auto b = std::get_if<BlockStmt>(&s->node)) {
    for (auto &x : b->statements)
      if (alwaysReturns(x))
        return true;
    return false;
  }
  if (auto i = std::get_if<IfStmt>(&s->node))
    return i->elseBranch && alwaysReturns(i->thenBranch) && alwaysReturns(i->elseBranch);
  return false;
}
} // namespace kyma
