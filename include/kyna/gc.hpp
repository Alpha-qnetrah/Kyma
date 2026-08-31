#pragma once
#include <cstddef>
#include <memory>
#include <vector>

namespace kyna {
struct Object;
struct Array;
class Environment;

struct HeapStats {
  std::size_t live{0};
  std::size_t allocated{0};
  std::size_t reclaimed{0};
  std::size_t collections{0};
  std::size_t peakLive{0};
  std::size_t nextThreshold{0};
};

// Tracing heap for runtime objects. Object fields are non-owning Value edges;
// the heap owns object storage and collects unreachable cycles automatically.
class Heap {
public:
  Heap() = default;
  ~Heap();
  Object *allocate();
  Array *allocateArray();
  void collect(const std::vector<Environment *> &roots);
  void maybeCollect(const std::vector<Environment *> &roots);
  void setThreshold(std::size_t threshold);
  std::size_t allocated() const { return allocatedCount; }
  std::size_t live() const { return objects.size() + arrays.size(); }
  std::size_t collections() const { return collectionCount; }
  HeapStats stats() const;

private:
  std::vector<std::unique_ptr<Object>> objects;
  std::vector<std::unique_ptr<Array>> arrays;
  std::size_t allocatedCount{0};
  std::size_t collectionCount{0};
  std::size_t reclaimedCount{0};
  std::size_t peakLiveCount{0};
  std::size_t minimumThreshold{256};
  std::size_t nextThreshold{256};
};
} // namespace kyna
