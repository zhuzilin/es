#ifndef ES_GC_HEAP_H
#define ES_GC_HEAP_H

#include <es/gc/copying_collection.h>
#include <es/gc/mark_and_sweep_collection.h>
#include <es/gc/no_collection.h>

namespace es {

constexpr size_t kNewSpaceSize = 4000U * 1024 * 1024;  // 500MB
constexpr size_t kConstantSegmentSize = 10 * 1024 * 1024;  // 10MB
constexpr size_t kBigObjectSegmentSize = 1024 * 1024 * 1024;  // 1GB
constexpr size_t kBigObjectThres = 10 * 1024 * 1024;  // 10MB

class Heap {
 public:
  static Heap* Global() {
    static Heap singleton;
    return &singleton;
  }

  void* Allocate(size_t size, flag_t flag) {
    if (flag & GCFlag::CONST)
      return constant_space_.New(size, flag);
    if (size > kBigObjectThres) {
      flag |= GCFlag::BIG;
      return big_object_space_.New(size, flag);
    }
    return new_space_.New(size, flag);
  }

 private:
  Heap() :
    new_space_(kNewSpaceSize),
    constant_space_(kConstantSegmentSize),
    big_object_space_(kBigObjectSegmentSize) {}

  CopyingCollection new_space_;
  // MarkAndSweepCollection new_space_;
  NoCollection constant_space_;
  NoCollection big_object_space_;
};

void* Allocate(size_t size, flag_t flag) {
  return Heap::Global()->Allocate(size, flag);
}

}  // namespace es


#endif  // ES_GC_HEAP_H