#include "kyma/runtime.hpp"
#include <algorithm>
#include <set>
#include <utility>

namespace kyma {
Heap::~Heap() = default;
Object *Heap::allocate() {
  objects.push_back(std::make_unique<Object>());
  ++allocatedCount;
  return objects.back().get();
}
Array *Heap::allocateArray() {
  arrays.push_back(std::make_unique<Array>());
  ++allocatedCount;
  return arrays.back().get();
}

void Heap::collect(const std::vector<Environment *> &roots) {
  std::set<Object *> marked;
  std::set<Array *> markedArrays;
  std::set<Environment *> visitedEnvironments;

  std::function<void(const Value &)> traceValue;
  std::function<void(const std::shared_ptr<Environment> &)> traceEnvironment;
  std::function<void(Object *)> traceObject;
  std::function<void(Array *)> traceArray;
  std::function<void(const FunctionPtr &)> traceFunction;
  std::function<void(const ClassPtr &)> traceClass;

  traceObject = [&](Object *object) {
    if (!object || !marked.insert(object).second)
      return;
    for (const auto &[name, value] : object->fields)
      traceValue(value);
  };
  traceArray = [&](Array *array) {
    if (!array || !markedArrays.insert(array).second)
      return;
    for (const auto &value : array->elements)
      traceValue(value);
  };
  traceEnvironment = [&](const std::shared_ptr<Environment> &environment) {
    if (!environment || !visitedEnvironments.insert(environment.get()).second)
      return;
    for (const auto &[name, cell] : environment->values)
      traceValue(cell.value);
    traceEnvironment(environment->enclosing);
  };
  traceFunction = [&](const FunctionPtr &function) {
    if (!function)
      return;
    traceObject(function->boundThis);
    traceEnvironment(function->closure);
  };
  traceClass = [&](const ClassPtr &klass) {
    if (!klass)
      return;
    for (const auto &[name, value] : klass->staticFields)
      traceValue(value);
    for (const auto &[name, function] : klass->methods)
      traceFunction(function);
    traceClass(klass->parent);
  };
  traceValue = [&](const Value &value) {
    if (const auto *object = std::get_if<ObjectPtr>(&value.data))
      traceObject(*object);
    else if (const auto *array = std::get_if<ArrayPtr>(&value.data))
      traceArray(*array);
    else if (const auto *function = std::get_if<FunctionPtr>(&value.data))
      traceFunction(*function);
    else if (const auto *klass = std::get_if<ClassPtr>(&value.data))
      traceClass(*klass);
  };

  for (auto *root : roots) {
    if (!root)
      continue;
    for (const auto &[name, cell] : root->values)
      traceValue(cell.value);
    traceEnvironment(root->enclosing);
  }

  objects.erase(std::remove_if(objects.begin(), objects.end(),
                               [&](const auto &object) { return !marked.contains(object.get()); }),
                objects.end());
  arrays.erase(
      std::remove_if(arrays.begin(), arrays.end(),
                     [&](const auto &array) { return !markedArrays.contains(array.get()); }),
      arrays.end());
  ++collectionCount;
  nextThreshold = std::max<std::size_t>(256, objects.size() * 2 + 1);
}

void Heap::maybeCollect(const std::vector<Environment *> &roots) {
  if (allocatedCount >= nextThreshold)
    collect(roots);
}

void Heap::setThreshold(std::size_t threshold) {
  nextThreshold = std::max<std::size_t>(1, threshold);
}
} // namespace kyma
