#include "kyna/runtime.hpp"

namespace kyna {
Environment::Environment(std::shared_ptr<Environment> p) : enclosing(std::move(p)) {}
void Environment::define(const std::string &name, Value value, bool mutableBinding) {
  if (values.contains(name))
    throw KynaError({"binding '" + name + "' is already declared in this scope", {1, 1}, false});
  values.emplace(name, Cell{std::move(value), mutableBinding});
}
Cell &Environment::get(const std::string &name) {
  if (auto found = values.find(name); found != values.end())
    return found->second;
  if (enclosing)
    return enclosing->get(name);
  throw KynaError({"undefined name '" + name + "'", {1, 1}, false});
}
void Environment::assign(const std::string &name, Value value) {
  auto found = values.find(name);
  if (found != values.end()) {
    if (!found->second.mutableBinding)
      throw KynaError({"cannot assign to immutable binding '" + name + "'", {1, 1}, false});
    found->second.value = std::move(value);
    return;
  }
  if (enclosing) {
    enclosing->assign(name, std::move(value));
    return;
  }
  throw KynaError({"undefined name '" + name + "'", {1, 1}, false});
}
std::shared_ptr<Environment> Environment::parent() const { return enclosing; }
} // namespace kyna
