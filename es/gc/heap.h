#ifndef ES_GC_HEAP_H
#define ES_GC_HEAP_H

#include <es/gc/copying_collection.h>
#include <es/gc/mark_and_sweep_collection.h>
#include <es/gc/no_collection.h>

namespace es {

constexpr size_t kNewSpaceSize = 4000U * 1024 * 1024;  // 4GB
constexpr size_t kConstantSegmentSize = 100 * 1024 * 1024;  // 100MB
constexpr size_t kBigObjectSegmentSize = 100 * 1024 * 1024;  // 100MB
constexpr size_t kBigObjectThres = 10 * 1024 * 1024;  // 10MB

class Heap {
 public:
  static Heap* Global() {
    static Heap singleton;
    return &singleton;
  }

  void* Allocate(size_t size_with_header, flag_t flag) {
    if (flag & GCFlag::CONST)
      return constant_space_.New(size_with_header, flag);
    if (unlikely(size_with_header > kBigObjectThres)) {
      flag |= GCFlag::BIG;
      return big_object_space_.New(size_with_header, flag);
    } else {
      return new_space_.New(size_with_header, flag);
    }
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

inline void* Allocate(size_t size_with_header, flag_t flag) {
  return Heap::Global()->Allocate(size_with_header, flag);
}

}  // namespace es


#endif  // ES_GC_HEAP_H