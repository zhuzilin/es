#ifndef ES_GC_BASE_COLLECTION_H
#define ES_GC_BASE_COLLECTION_H

#include <stdlib.h>
#include <sys/time.h>

#include <es/runtime.h>

namespace es {

constexpr size_t kAlignmentBits = 3;
constexpr size_t kAlignmentMask = (1 << kAlignmentBits) - 1;

template<typename T>
class GC {
 public:
  void* New(size_t size_with_header, flag_t flag) {
    if (unlikely(size_with_header & kAlignmentMask)) {
      size_with_header = ((size_with_header >> 3) + 1) << 3;
    }
    void* ref = Allocate(size_with_header, flag);
    if (ref == nullptr) {
#ifdef TIMER
      struct timeval start, end;
      gettimeofday(&start, nullptr);
#endif
      Collect();
#ifdef TIMER
      gettimeofday(&end, nullptr);
      time += (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
      std::cout << "Total Garbage Collection time: " << time << " ms\n";
#endif
      ref = Allocate(size_with_header, flag);
      if (ref == nullptr) {
        throw std::runtime_error("Out of memory");
      }
    }
    return ref;
  }

 private:
  void* Allocate(size_t size, flag_t flag) {
    return static_cast<T*>(this)->AllocateImpl(size, flag);
  }

  void Collect() {
    CleanUpBeforeCollect();
    static_cast<T*>(this)->CollectImpl();
  }

  void CleanUpBeforeCollect();

#ifdef TIMER
  double time = 0;
#endif
};

}  // namespace es

#endif  // ES_GC_BASE_COLLECTION_H