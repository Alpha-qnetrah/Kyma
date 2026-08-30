#pragma once
#include "kyma/ast.hpp"
#include "kyma/gc.hpp"
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace kyma {
struct Object; struct Array; struct Function; struct Class;
using ObjectPtr = Object*; using ArrayPtr = Array*; using FunctionPtr = std::shared_ptr<Function>; using ClassPtr = std::shared_ptr<Class>;
struct Value {
  using Data = std::variant<std::nullptr_t, bool, int64_t, double, std::string, char, ObjectPtr, ArrayPtr, FunctionPtr, ClassPtr>;
  Data data{nullptr};
  Value() = default; template<class T> Value(T v): data(std::move(v)) {}
  std::string typeName() const; std::string display() const; bool isTruthy() const; bool equals(const Value&) const;
};
struct Cell { Value value; bool mutableBinding{false}; };
class Environment : public std::enable_shared_from_this<Environment> {
public: explicit Environment(std::shared_ptr<Environment> parent = nullptr); void define(const std::string&, Value, bool); Cell& get(const std::string&); void assign(const std::string&, Value); std::shared_ptr<Environment> parent() const;
private: std::map<std::string, Cell> values; std::shared_ptr<Environment> enclosing; friend class Heap;
};
struct Object { std::map<std::string, Value> fields; ClassPtr klass; };
struct Array { std::vector<Value> elements; };
struct Function { FunctionDecl declaration; std::shared_ptr<Environment> closure; ObjectPtr boundThis; bool native{false}; std::function<Value(const std::vector<Value>&)> nativeCall; Value call(const std::vector<Value>&, class Interpreter&); };
struct Class { ClassDecl declaration; ClassPtr parent; std::map<std::string, FunctionPtr> methods; std::map<std::string, Value> staticFields; FunctionPtr findMethod(const std::string&) const; };
}
