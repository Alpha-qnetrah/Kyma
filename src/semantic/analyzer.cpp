#include "kyna/analyzer.hpp"
#include "kyna/behavior.hpp"
#include <algorithm>
#include <map>

namespace kyna {
namespace {
TypeRef t(const std::string &n) { return TypeRef{n, false, {}}; }
int visibility(const std::vector<std::string> &modifiers) {
  if (hasModifier(modifiers, "public"))
    return 2;
  if (hasModifier(modifiers, "protected"))
    return 1;
  return 0;
}
bool sameParameters(const FunctionDecl &left, const FunctionDecl &right) {
  if (left.params.size() != right.params.size())
    return false;
  for (std::size_t index = 0; index < left.params.size(); ++index)
    if (left.params[index].type.str() != right.params[index].type.str())
      return false;
  return true;
}
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
         n == "readJsonFile" || n == "writeJsonFile" || n == "createDirectory" ||
         n == "fileExists" || n == "removePath" || n == "listDirectory" || n == "fs" ||
         n == "processRun" || n == "processEnv" || n == "sleep" || n == "httpGet" || n == "fetch" ||
         n == "build" || n == "wait" || n == "log" || n == "logColor" || n == "console" ||
         n == "error" || n == "filter" || n == "sort" || n == "bubbleSort" || n == "call" ||
         n == "jsonParse" || n == "jsonStringify" || n == "process" || n == "createApiStore";
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
  if (const auto *contract = interfaces.find(e.name); contract && classes.contains(a.name))
    return classConforms(classes[a.name], *contract, {});
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
  for (const auto &[name, type] : externalBindings) {
    scope->types[name] = type;
    scope->mutableBindings[name] = false;
  }
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
      [this, &s](const auto &n) {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, VarDecl>) {
          for (auto p = scope->parent; p; p = p->parent)
            if (p->types.contains(n.name)) {
              warning("binding '" + n.name + "' shadows an outer binding", SourceLocation{});
              break;
            }
          if (n.initializer) {
            auto a = expr(n.initializer);
            if (n.hasType) {
              if (const auto *contract = interfaces.find(n.type.name))
                if (const auto *object = std::get_if<ObjectExpr>(&n.initializer->node))
                  objectConforms(*object, *contract, n.initializer->location);
            }
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
          if (hasModifier(n.modifiers, "abstract") && hasModifier(n.modifiers, "final"))
            error("class '" + n.name + "' cannot be both abstract and final", s->location);
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
                error("override return type for '" + m.name + "' is incompatible", s->location);
              if (inherited != base.methods.end() && !sameParameters(m, *inherited))
                error("override parameters for '" + m.name + "' must match the inherited method",
                      s->location);
              if (inherited != base.methods.end() &&
                  visibility(m.modifiers) < visibility(inherited->modifiers))
                error("override of '" + m.name + "' cannot narrow visibility", s->location);
            }
          }
          for (const auto &contractName : n.interfaces) {
            const auto *contract = interfaces.find(contractName);
            if (!contract)
              error("unknown interface '" + contractName + "'", s->location);
            else
              classConforms(n, *contract, s->location);
          }
          const bool abstractClass = hasModifier(n.modifiers, "abstract");
          for (auto &m : n.methods) {
            const bool abstractMethod = hasModifier(m.modifiers, "abstract");
            if (abstractMethod && !abstractClass)
              error("abstract method '" + m.name + "' requires an abstract class", s->location);
            if (abstractMethod && m.body)
              error("abstract method '" + m.name + "' cannot have a body", s->location);
            if (!abstractMethod && !m.body)
              error("concrete method '" + m.name + "' requires a body", s->location);
            if (!m.body)
              continue;
            auto old = scope;
            auto oldReturn = currentReturn;
            bool oldIn = inFunction;
            auto oldClass = currentClass;
            currentClass = n.name;
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
            currentClass = std::move(oldClass);
          }
          if (!abstractClass && !n.parent.empty() && classes.contains(n.parent)) {
            for (const auto &inherited : classes[n.parent].methods) {
              if (!hasModifier(inherited.modifiers, "abstract"))
                continue;
              const auto implementation =
                  std::find_if(n.methods.begin(), n.methods.end(), [&](const FunctionDecl &method) {
                    return method.name == inherited.name;
                  });
              if (implementation == n.methods.end() ||
                  hasModifier(implementation->modifiers, "abstract"))
                error("concrete class '" + n.name + "' must implement abstract method '" +
                          inherited.name + "'",
                      s->location);
            }
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
const FieldDecl *Analyzer::findField(const ClassDecl &klass, const std::string &name) const {
  const auto found = std::find_if(klass.fields.begin(), klass.fields.end(),
                                  [&](const auto &field) { return field.name == name; });
  if (found != klass.fields.end())
    return &*found;
  if (!klass.parent.empty()) {
    const auto parent = classes.find(klass.parent);
    if (parent != classes.end())
      return findField(parent->second, name);
  }
  return nullptr;
}
const FunctionDecl *Analyzer::findMethod(const ClassDecl &klass, const std::string &name) const {
  const auto found = std::find_if(klass.methods.begin(), klass.methods.end(),
                                  [&](const auto &method) { return method.name == name; });
  if (found != klass.methods.end())
    return &*found;
  if (!klass.parent.empty()) {
    const auto parent = classes.find(klass.parent);
    if (parent != classes.end())
      return findMethod(parent->second, name);
  }
  return nullptr;
}
bool Analyzer::classConforms(const ClassDecl &klass, const InterfaceDecl &contract,
                             SourceLocation location) {
  bool conforms = true;
  for (const auto &required : contract.fields) {
    const auto *field = findField(klass, required.name);
    if (!field || !compatible(required.type, field->type)) {
      conforms = false;
      if (location.known())
        error("class '" + klass.name + "' does not provide compatible field '" + required.name +
                  "' required by interface '" + contract.name + "'",
              location);
    }
  }
  for (const auto &required : contract.methods) {
    const auto *method = findMethod(klass, required.name);
    if (!method || !sameParameters(*method, required) ||
        !compatible(required.returnType, method->returnType) ||
        visibility(method->modifiers) != 2) {
      conforms = false;
      if (location.known())
        error("class '" + klass.name + "' does not provide compatible public method '" +
                  required.name + "' required by interface '" + contract.name + "'",
              location);
    }
  }
  return conforms;
}
bool Analyzer::objectConforms(const ObjectExpr &object, const InterfaceDecl &contract,
                              SourceLocation location) {
  bool conforms = true;
  for (const auto &required : contract.fields) {
    const auto found = std::find_if(object.fields.begin(), object.fields.end(),
                                    [&](const auto &f) { return f.name == required.name; });
    if (found == object.fields.end() || !compatible(required.type, expr(found->value))) {
      conforms = false;
      error("object does not provide compatible field '" + required.name +
                "' required by interface '" + contract.name + "'",
            location);
    }
  }
  if (!contract.methods.empty()) {
    conforms = false;
    error("closed object literals cannot provide methods required by interface '" + contract.name +
              "'",
          location);
  }
  return conforms;
}
} // namespace kyna
