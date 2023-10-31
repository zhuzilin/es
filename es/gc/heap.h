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

  template<size_t size_with_header, flag_t flag>
  void* Allocate() {
    if constexpr (flag & GCFlag::CONST)
      return constant_space_.New<size_with_header, flag>();
    if constexpr (size_with_header > kBigObjectThres) {
      return big_object_space_.New<size_with_header, flag | GCFlag::BIG>();
    } else {
      return new_space_.New<size_with_header, flag>();
    }
  }

  template<flag_t flag>
  void* Allocate(size_t size_with_header) {
    if constexpr (flag & GCFlag::CONST)
      return constant_space_.New<flag>(size_with_header);
    if (unlikely(size_with_header > kBigObjectThres)) {
      return big_object_space_.New<flag | GCFlag::BIG>(size_with_header);
    } else {
      return new_space_.New<flag>(size_with_header);
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

template<uint32_t size_with_header, flag_t flag>
inline void* Allocate() {
  return Heap::Global()->Allocate<size_with_header, flag>();
}

template<flag_t flag>
inline void* Allocate(uint32_t size_with_header) {
  return Heap::Global()->Allocate<flag>(size_with_header);
}

}  // namespace es


#endif  // ES_GC_HEAP_H