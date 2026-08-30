#pragma once
#include <cstddef>
#include <memory>
#include <vector>

namespace kyma {
struct Object;
class Environment;

// Tracing heap for runtime objects. Object fields are non-owning Value edges;
// the heap owns object storage and collects unreachable cycles automatically.
class Heap {
public:
  Heap() = default;
  ~Heap();
  Object* allocate();
  void collect(const std::vector<Environment*>& roots);
  void maybeCollect(const std::vector<Environment*>& roots);
  void setThreshold(std::size_t threshold);
  std::size_t allocated() const { return allocatedCount; }
  std::size_t live() const { return objects.size(); }
  std::size_t collections() const { return collectionCount; }
private:
  std::vector<std::unique_ptr<Object>> objects;
  std::size_t allocatedCount{0};
  std::size_t collectionCount{0};
  std::size_t nextThreshold{256};
};
}
