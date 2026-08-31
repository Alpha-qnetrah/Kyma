#include "kyna/behavior.hpp"
#include "kyna/interpreter.hpp"

namespace kyna {
namespace {

std::string decodeQuotedLiteral(const std::string &literal) {
  std::string value;
  const auto end = literal.size() > 1 ? literal.size() - 1 : literal.size();
  for (std::size_t index = 1; index < end; ++index) {
    if (literal[index] != '\\' || index + 1 >= end) {
      value.push_back(literal[index]);
      continue;
    }
    switch (literal[++index]) {
    case 'n':
      value.push_back('\n');
      break;
    case 'r':
      value.push_back('\r');
      break;
    case 't':
      value.push_back('\t');
      break;
    case 'b':
      value.push_back('\b');
      break;
    case 'f':
      value.push_back('\f');
      break;
    case '\\':
      value.push_back('\\');
      break;
    case '"':
      value.push_back('"');
      break;
    case '\'':
      value.push_back('\'');
      break;
    default:
      value.push_back(literal[index]);
      break;
    }
  }
  return value;
}

} // namespace

Value Interpreter::eval(const ExprPtr &e) {
  return std::visit(
      [this](const auto &n) -> Value {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, Literal>) {
          switch (n.kind) {
          case Literal::Kind::Null:
            return Value();
          case Literal::Kind::Bool:
            return Value(n.value == "true");
          case Literal::Kind::Int:
            return Value(static_cast<int64_t>(std::stoll(n.value)));
          case Literal::Kind::Float:
            return Value(std::stod(n.value));
          case Literal::Kind::String: {
            return Value(decodeQuotedLiteral(n.value));
          }
          case Literal::Kind::Char: {
            const auto value = decodeQuotedLiteral(n.value);
            return Value(value.empty() ? '\0' : value.front());
          }
          }
        } else if constexpr (std::is_same_v<T, Variable>)
          return environment->get(n.name).value;
        else if constexpr (std::is_same_v<T, SelfExpr>)
          return environment->get("self").value;
        else if constexpr (std::is_same_v<T, SuperExpr>)
          return environment->get("self").value;
        else if constexpr (std::is_same_v<T, Unary>) {
          auto v = eval(n.right);
          if (n.op == TokenKind::Bang)
            return Value(!v.isTruthy());
          if (auto i = std::get_if<int64_t>(&v.data))
            return Value(-*i);
          if (auto d = std::get_if<double>(&v.data))
            return Value(-*d);
          throw KynaError({"unary '-' requires a number", {1, 1}, false});
        } else if constexpr (std::is_same_v<T, Binary>) {
          if (n.op == TokenKind::AndAnd) {
            auto l = eval(n.left);
            return l.isTruthy() ? Value(eval(n.right).isTruthy()) : Value(false);
          }
          if (n.op == TokenKind::OrOr) {
            auto l = eval(n.left);
            return l.isTruthy() ? Value(true) : Value(eval(n.right).isTruthy());
          }
          return binary(n.op, eval(n.left), eval(n.right));
        } else if constexpr (std::is_same_v<T, Assign>) {
          Value v = eval(n.value);
          if (auto x = std::get_if<Variable>(&n.target->node))
            environment->assign(x->name, v);
          else if (auto m = std::get_if<Member>(&n.target->node))
            setMember(m->object, m->name, v);
          else if (auto i = std::get_if<Index>(&n.target->node))
            setIndex(i->object, i->index, v);
          else
            throw KynaError({"invalid assignment target", {1, 1}, false});
          return v;
        } else if constexpr (std::is_same_v<T, Call>)
          return call(n, n.callee);
        else if constexpr (std::is_same_v<T, Member>)
          return getMember(n);
        else if constexpr (std::is_same_v<T, Index>) {
          auto object = eval(n.object);
          auto index = eval(n.index);
          if (!std::holds_alternative<ArrayPtr>(object.data) ||
              !std::holds_alternative<int64_t>(index.data))
            throw KynaError({"array indexing requires an integer index", {1, 1}, false});
          auto array = std::get<ArrayPtr>(object.data);
          auto i = std::get<int64_t>(index.data);
          if (i < 0 || static_cast<size_t>(i) >= array->elements.size())
            throw KynaError({"array index out of bounds", {1, 1}, false});
          return array->elements[static_cast<size_t>(i)];
        } else if constexpr (std::is_same_v<T, ArrayExpr>) {
          auto array = objectHeap.allocateArray();
          for (auto &element : n.elements)
            array->elements.push_back(eval(element));
          return Value(array);
        } else if constexpr (std::is_same_v<T, NewExpr>) {
          auto v = environment->get(n.className).value;
          if (!std::holds_alternative<ClassPtr>(v.data))
            throw KynaError({"'" + n.className + "' is not a class", {1, 1}, false});
          auto c = std::get<ClassPtr>(v.data);
          if (hasModifier(c->declaration.modifiers, "abstract"))
            throw KynaError(
                {"cannot instantiate abstract class '" + n.className + "'", {1, 1}, false});
          auto o = objectHeap.allocate();
          o->klass = c;
          std::function<void(const ClassPtr &)> addFields = [&](const ClassPtr &k) {
            if (k->parent)
              addFields(k->parent);
            for (auto &f : k->declaration.fields)
              if (!hasModifier(f.modifiers, "static"))
                o->fields[f.name] = f.initializer ? eval(f.initializer) : Value();
          };
          addFields(c);
          auto init = c->findMethod("init");
          if (init)
            invoke(
                init,
                [&] {
                  std::vector<Value> a;
                  for (auto &x : n.args)
                    a.push_back(eval(x));
                  return a;
                }(),
                o);
          else if (!n.args.empty())
            throw KynaError({"constructor takes no arguments", {1, 1}, false});
          return Value(o);
        } else if constexpr (std::is_same_v<T, ObjectExpr>) {
          auto o = objectHeap.allocate();
          for (auto &f : n.fields)
            o->fields[f.name] = eval(f.value);
          return Value(o);
        } else if constexpr (std::is_same_v<T, IfExpr>) {
          auto b = eval(n.condition).isTruthy() ? n.thenBranch : n.elseBranch;
          auto *p = std::get_if<BlockStmt>(&b->node);
          if (!p) {
            exec(b);
            return Value();
          }
          auto old = environment;
          environment = std::make_shared<Environment>(environment);
          for (auto &s : p->statements)
            exec(s);
          Value r = p->tail ? eval(p->tail) : Value();
          environment = old;
          return r;
        } else if constexpr (std::is_same_v<T, MatchExpr>) {
          auto subject = eval(n.subject);
          for (auto &a : n.arms)
            if (a.wildcard || subject.equals(eval(a.pattern)))
              return eval(a.value);
          throw KynaError({"non-exhaustive match at runtime", {1, 1}, false});
        } else
          return Value();
      },
      e->node);
}
Value Interpreter::binary(TokenKind op, const Value &a, const Value &b) {
  if (op == TokenKind::EqualEqual)
    return Value(a.equals(b));
  if (op == TokenKind::BangEqual)
    return Value(!a.equals(b));
  if (op == TokenKind::Plus) {
    if (std::holds_alternative<std::string>(a.data) || std::holds_alternative<std::string>(b.data))
      return Value(a.display() + b.display());
    if (std::holds_alternative<int64_t>(a.data) && std::holds_alternative<int64_t>(b.data))
      return Value(std::get<int64_t>(a.data) + std::get<int64_t>(b.data));
    if ((std::holds_alternative<int64_t>(a.data) || std::holds_alternative<double>(a.data)) &&
        (std::holds_alternative<int64_t>(b.data) || std::holds_alternative<double>(b.data)))
      return Value((std::holds_alternative<int64_t>(a.data) ? std::get<int64_t>(a.data)
                                                            : std::get<double>(a.data)) +
                   (std::holds_alternative<int64_t>(b.data) ? std::get<int64_t>(b.data)
                                                            : std::get<double>(b.data)));
    throw KynaError({"'+' requires numbers or strings", {1, 1}, false});
  }
  bool nums = (std::holds_alternative<int64_t>(a.data) || std::holds_alternative<double>(a.data)) &&
              (std::holds_alternative<int64_t>(b.data) || std::holds_alternative<double>(b.data));
  if (!nums)
    throw KynaError({"comparison requires numbers", {1, 1}, false});
  double x = std::holds_alternative<int64_t>(a.data) ? std::get<int64_t>(a.data)
                                                     : std::get<double>(a.data),
         y = std::holds_alternative<int64_t>(b.data) ? std::get<int64_t>(b.data)
                                                     : std::get<double>(b.data);
  switch (op) {
  case TokenKind::Minus:
    return Value(x - y);
  case TokenKind::Star:
    return Value(x * y);
  case TokenKind::Slash:
    if (y == 0)
      throw KynaError({"division by zero", {1, 1}, false});
    return Value(x / y);
  case TokenKind::Percent:
    return Value(static_cast<int64_t>(x) % static_cast<int64_t>(y));
  case TokenKind::Less:
    return Value(x < y);
  case TokenKind::LessEqual:
    return Value(x <= y);
  case TokenKind::Greater:
    return Value(x > y);
  case TokenKind::GreaterEqual:
    return Value(x >= y);
  default:
    throw KynaError({"unsupported operator", {1, 1}, false});
  }
}
Value Interpreter::call(const Call &c, const ExprPtr &callee) {
  Value v = eval(callee);
  std::vector<Value> a;
  for (auto &e : c.args)
    a.push_back(eval(e));
  if (!std::holds_alternative<FunctionPtr>(v.data))
    throw KynaError({"value of type '" + v.typeName() + "' is not callable", {1, 1}, false});
  return invoke(std::get<FunctionPtr>(v.data), a, std::get<FunctionPtr>(v.data)->boundThis);
}
Value Interpreter::getMember(const Member &m) {
  if (std::holds_alternative<SuperExpr>(m.object->node)) {
    auto self = environment->get("self").value;
    auto o = std::get<ObjectPtr>(self.data);
    auto pc = environment->get("__parent_class").value;
    auto c = std::get<ClassPtr>(pc.data);
    auto f = c->findMethod(m.name);
    if (!f)
      throw KynaError({"parent has no member '" + m.name + "'", {1, 1}, false});
    auto bound = std::make_shared<Function>(*f);
    bound->boundThis = o;
    return Value(bound);
  }
  Value obj = eval(m.object);
  if (auto o = std::get_if<ObjectPtr>(&obj.data)) {
    if (auto f = (*o)->fields.find(m.name); f != (*o)->fields.end())
      return f->second;
    auto f = (*o)->klass ? (*o)->klass->findMethod(m.name) : nullptr;
    if (f) {
      auto bound = std::make_shared<Function>(*f);
      bound->boundThis = *o;
      return Value(bound);
    }
  }
  if (auto c = std::get_if<ClassPtr>(&obj.data)) {
    if (auto x = (*c)->staticFields.find(m.name); x != (*c)->staticFields.end())
      return x->second;
    if (auto f = (*c)->findMethod(m.name); f && hasModifier(f->declaration.modifiers, "static"))
      return Value(f);
  }
  if (auto module = std::get_if<ModulePtr>(&obj.data)) {
    if (!*module || !(*module)->exports.contains(m.name))
      throw KynaError(
          {"module has no exported member '" + m.name + "'", m.object->location, false, "K5008"});
    return (*module)->environment->get(m.name).value;
  }
  throw KynaError({"unknown member '" + m.name + "'", {1, 1}, false});
}
void Interpreter::setIndex(const ExprPtr &o, const ExprPtr &i, Value v) {
  auto object = eval(o);
  auto index = eval(i);
  if (!std::holds_alternative<ArrayPtr>(object.data) ||
      !std::holds_alternative<int64_t>(index.data))
    throw KynaError({"array indexing requires an integer index", {1, 1}, false});
  auto array = std::get<ArrayPtr>(object.data);
  auto position = std::get<int64_t>(index.data);
  if (position < 0 || static_cast<size_t>(position) >= array->elements.size())
    throw KynaError({"array index out of bounds", {1, 1}, false});
  array->elements[static_cast<size_t>(position)] = std::move(v);
}
void Interpreter::setMember(const ExprPtr &o, const std::string &n, Value v) {
  auto obj = eval(o);
  if (!std::holds_alternative<ObjectPtr>(obj.data))
    throw KynaError({"member assignment requires an object", {1, 1}, false});
  auto x = std::get<ObjectPtr>(obj.data);
  if (!x->fields.contains(n))
    throw KynaError({"unknown field '" + n + "' on closed object", {1, 1}, false});
  x->fields[n] = std::move(v);
}
} // namespace kyna
