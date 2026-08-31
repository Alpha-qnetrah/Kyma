#include "kyma/runtime.hpp"
#include <algorithm>
#include <deque>
#include <set>

namespace kyma {

Heap::~Heap() = default;

Object *Heap::allocate() {
  objects.push_back(std::make_unique<Object>());
  ++allocatedCount;
  peakLiveCount = std::max(peakLiveCount, live());
  return objects.back().get();
}

Array *Heap::allocateArray() {
  arrays.push_back(std::make_unique<Array>());
  ++allocatedCount;
  peakLiveCount = std::max(peakLiveCount, live());
  return arrays.back().get();
}

void Heap::collect(const std::vector<Environment *> &roots) {
  const auto before = live();
  std::set<Object *> markedObjects;
  std::set<Array *> markedArrays;
  std::set<Environment *> markedEnvironments;
  std::set<Function *> markedFunctions;
  std::set<Class *> markedClasses;
  std::set<ModuleNamespace *> markedModules;
  std::deque<Value> pendingValues;
  std::deque<Environment *> pendingEnvironments;

  for (auto *root : roots)
    if (root)
      pendingEnvironments.push_back(root);

  while (!pendingEnvironments.empty() || !pendingValues.empty()) {
    while (!pendingEnvironments.empty()) {
      auto *environment = pendingEnvironments.front();
      pendingEnvironments.pop_front();
      if (!environment || !markedEnvironments.insert(environment).second)
        continue;
      for (const auto &[name, cell] : environment->values)
        pendingValues.push_back(cell.value);
      if (environment->enclosing)
        pendingEnvironments.push_back(environment->enclosing.get());
    }
    if (pendingValues.empty())
      continue;
    auto value = std::move(pendingValues.front());
    pendingValues.pop_front();
    if (const auto *object = std::get_if<ObjectPtr>(&value.data)) {
      if (!*object || !markedObjects.insert(*object).second)
        continue;
      for (const auto &[name, field] : (*object)->fields)
        pendingValues.push_back(field);
      if ((*object)->klass)
        pendingValues.emplace_back((*object)->klass);
    } else if (const auto *array = std::get_if<ArrayPtr>(&value.data)) {
      if (!*array || !markedArrays.insert(*array).second)
        continue;
      for (const auto &element : (*array)->elements)
        pendingValues.push_back(element);
    } else if (const auto *function = std::get_if<FunctionPtr>(&value.data)) {
      if (!*function || !markedFunctions.insert(function->get()).second)
        continue;
      if ((*function)->boundThis)
        pendingValues.emplace_back((*function)->boundThis);
      if ((*function)->closure)
        pendingEnvironments.push_back((*function)->closure.get());
    } else if (const auto *klass = std::get_if<ClassPtr>(&value.data)) {
      if (!*klass || !markedClasses.insert(klass->get()).second)
        continue;
      if ((*klass)->parent)
        pendingValues.emplace_back((*klass)->parent);
      for (const auto &[name, field] : (*klass)->staticFields)
        pendingValues.push_back(field);
      for (const auto &[name, method] : (*klass)->methods)
        pendingValues.emplace_back(method);
    } else if (const auto *module = std::get_if<ModulePtr>(&value.data)) {
      if (!*module || !markedModules.insert(module->get()).second)
        continue;
      if ((*module)->environment)
        pendingEnvironments.push_back((*module)->environment.get());
    }
  }

  objects.erase(
      std::remove_if(objects.begin(), objects.end(),
                     [&](const auto &object) { return !markedObjects.contains(object.get()); }),
      objects.end());
  arrays.erase(
      std::remove_if(arrays.begin(), arrays.end(),
                     [&](const auto &array) { return !markedArrays.contains(array.get()); }),
      arrays.end());
  reclaimedCount += before - live();
  ++collectionCount;
  nextThreshold = std::max(minimumThreshold, live() * 2 + 1);
}

void Heap::maybeCollect(const std::vector<Environment *> &roots) {
  if (live() >= nextThreshold)
    collect(roots);
}

void Heap::setThreshold(std::size_t threshold) {
  minimumThreshold = std::max<std::size_t>(1, threshold);
  nextThreshold = minimumThreshold;
}

HeapStats Heap::stats() const {
  return {live(), allocatedCount, reclaimedCount, collectionCount, peakLiveCount, nextThreshold};
}

} // namespace kyma
