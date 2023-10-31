#ifndef ES_GC_BASE_COLLECTION_H
#define ES_GC_BASE_COLLECTION_H

#include <stdlib.h>
#include <sys/time.h>

#include <es/runtime.h>

namespace es {

constexpr size_t kAlignmentBits = 3;
constexpr size_t kAlignmentMask = (1 << kAlignmentBits) - 1;

constexpr uint32_t align(uint32_t size) {
  return (size & kAlignmentMask) ? ((size >> 3) + 1) << 3 : size;
}

template<typename T>
class GC {
 public:
  template<uint32_t size_with_header, flag_t flag>
  void* New() {
    void* ref = Allocate<align(size_with_header), flag>();
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
      ref = Allocate<align(size_with_header), flag>();
      if (ref == nullptr) {
        throw std::runtime_error("Out of memory");
      }
    }
    return ref;
  }

  template<flag_t flag>
  void* New(uint32_t size_with_header) {
    if (size_with_header & kAlignmentMask) {
      size_with_header = ((size_with_header >> 3) + 1) << 3;
    }
    void* ref = Allocate<flag>(size_with_header);
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
      ref = Allocate<flag>(size_with_header);
      if (ref == nullptr) {
        throw std::runtime_error("Out of memory");
      }
    }
    return ref;
  }

 private:
  template<size_t size, flag_t flag>
  void* Allocate() {
    return static_cast<T*>(this)->template AllocateImpl<size, flag>();
  }

  template<flag_t flag>
  void* Allocate(size_t size) {
    return static_cast<T*>(this)->template AllocateImpl<flag>(size);
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