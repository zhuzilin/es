#ifndef ES_GC_BASE_COLLECTION_H
#define ES_GC_BASE_COLLECTION_H

#include <stdlib.h>
#include <sys/time.h>

#include <es/runtime.h>

namespace es {

class GC {
 public:
  void* New(size_t size, flag_t flag) {
    size_t size_with_header = size + sizeof(Header);
    if (size_with_header % 8 != 0) {
      size_with_header += 8 - size_with_header % 8;
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
      std::cout << "Garbage Collection time: " << time << "\n";
#endif
      ref = Allocate(size_with_header, flag);
      if (ref == nullptr) {
        throw std::runtime_error("Out of memory");
      }
    }
    void* body = static_cast<Header*>(ref) + 1;
    return body;
  }

  void* ForwardAddress(void* ref) {
    Header* header = H(ref);
    return header->forward_address;
  }

  void SetForwardAddress(void* ref, void* forward_address) {
    Header* header = H(ref);
    header->forward_address = forward_address;
  }

 private:
  virtual void* Allocate(size_t size, flag_t flag) = 0;
  virtual void Collect() = 0;

#ifdef TIMER
  double time = 0;
#endif
};

}  // namespace es

#endif  // ES_GC_BASE_COLLECTION_H