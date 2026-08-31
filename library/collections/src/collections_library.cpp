#include "kyna/stdlib/collections_library.hpp"

#include "kyna/execution/tree_walk_engine.hpp"

namespace kyna {
namespace {

std::pair<ArrayPtr, FunctionPtr> arrayAndFunction(const std::vector<Value> &arguments,
                                                  const std::string &name) {
  if (arguments.size() != 2 || !std::holds_alternative<ArrayPtr>(arguments[0].data) ||
      !std::holds_alternative<FunctionPtr>(arguments[1].data))
    throw KynaError({name + " expects an array and function", {}, false, "KCOL1001"});
  return {std::get<ArrayPtr>(arguments[0].data), std::get<FunctionPtr>(arguments[1].data)};
}

std::vector<Value> elementArguments(const FunctionPtr &function, const Value &value,
                                    std::size_t index) {
  std::vector<Value> arguments{value};
  if (!function->native && function->declaration.params.size() > 1)
    arguments.emplace_back(static_cast<std::int64_t>(index));
  return arguments;
}

} // namespace

void installCollectionsLibrary(Interpreter &interpreter) {
  auto transform = std::make_shared<Function>();
  transform->native = true;
  transform->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    const auto [input, mapper] = arrayAndFunction(arguments, "map");
    auto output = interpreter.heap().allocateArray();
    output->elements.reserve(input->elements.size());
    for (std::size_t index = 0; index < input->elements.size(); ++index)
      output->elements.push_back(
          mapper->call(elementArguments(mapper, input->elements[index], index), interpreter));
    return Value(output);
  };
  interpreter.globals()->define("map", Value(transform), false);

  auto reduce = std::make_shared<Function>();
  reduce->native = true;
  reduce->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.size() != 3 || !std::holds_alternative<ArrayPtr>(arguments[0].data) ||
        !std::holds_alternative<FunctionPtr>(arguments[1].data))
      throw KynaError(
          {"reduce expects an array, reducer function, and initial value", {}, false, "KCOL1002"});
    const auto input = std::get<ArrayPtr>(arguments[0].data);
    const auto reducer = std::get<FunctionPtr>(arguments[1].data);
    Value accumulated = arguments[2];
    for (std::size_t index = 0; index < input->elements.size(); ++index) {
      std::vector<Value> reducerArguments{accumulated, input->elements[index]};
      if (!reducer->native && reducer->declaration.params.size() > 2)
        reducerArguments.emplace_back(static_cast<std::int64_t>(index));
      accumulated = reducer->call(reducerArguments, interpreter);
    }
    return accumulated;
  };
  interpreter.globals()->define("reduce", Value(reduce), false);

  auto find = std::make_shared<Function>();
  find->native = true;
  find->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    const auto [input, predicate] = arrayAndFunction(arguments, "find");
    for (std::size_t index = 0; index < input->elements.size(); ++index)
      if (predicate->call(elementArguments(predicate, input->elements[index], index), interpreter)
              .isTruthy())
        return input->elements[index];
    return Value();
  };
  interpreter.globals()->define("find", Value(find), false);

  auto any = std::make_shared<Function>();
  any->native = true;
  any->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    const auto [input, predicate] = arrayAndFunction(arguments, "any");
    for (std::size_t index = 0; index < input->elements.size(); ++index)
      if (predicate->call(elementArguments(predicate, input->elements[index], index), interpreter)
              .isTruthy())
        return Value(true);
    return Value(false);
  };
  interpreter.globals()->define("any", Value(any), false);

  auto all = std::make_shared<Function>();
  all->native = true;
  all->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    const auto [input, predicate] = arrayAndFunction(arguments, "all");
    for (std::size_t index = 0; index < input->elements.size(); ++index)
      if (!predicate->call(elementArguments(predicate, input->elements[index], index), interpreter)
               .isTruthy())
        return Value(false);
    return Value(true);
  };
  interpreter.globals()->define("all", Value(all), false);

  auto unique = std::make_shared<Function>();
  unique->native = true;
  unique->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<ArrayPtr>(arguments[0].data))
      throw KynaError({"unique expects one array", {}, false, "KCOL1003"});
    auto output = interpreter.heap().allocateArray();
    for (const auto &candidate : std::get<ArrayPtr>(arguments[0].data)->elements) {
      bool exists = false;
      for (const auto &accepted : output->elements)
        if (accepted.equals(candidate)) {
          exists = true;
          break;
        }
      if (!exists)
        output->elements.push_back(candidate);
    }
    return Value(output);
  };
  interpreter.globals()->define("unique", Value(unique), false);

  auto collections = interpreter.heap().allocate();
  collections->fields["map"] = Value(transform);
  collections->fields["reduce"] = Value(reduce);
  collections->fields["find"] = Value(find);
  collections->fields["any"] = Value(any);
  collections->fields["all"] = Value(all);
  collections->fields["unique"] = Value(unique);
  interpreter.globals()->define("collections", Value(collections), false);
}

} // namespace kyna
