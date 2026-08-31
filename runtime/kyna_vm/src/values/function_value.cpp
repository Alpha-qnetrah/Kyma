#include "kyna/execution/tree_walk_engine.hpp"

namespace kyna {
FunctionPtr Class::findMethod(const std::string &name) const {
  if (auto found = methods.find(name); found != methods.end())
    return found->second;
  return parent ? parent->findMethod(name) : nullptr;
}
Value Function::call(const std::vector<Value> &args, Interpreter &interpreter) {
  if (native)
    return nativeCall(args);
  return interpreter.invoke(std::make_shared<Function>(*this), args, boundThis);
}
} // namespace kyna
