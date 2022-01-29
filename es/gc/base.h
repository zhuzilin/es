#ifndef ES_GC_BASE_H
#define ES_GC_BASE_H

#include <stdlib.h>

#include <es/runtime.h>

namespace es {

class GC {
 public:
  void* New(size_t size, flag_t flag) {
    size_t size_with_header = size + sizeof(Header);
    if (size_with_header % 8 != 0) {
      size_with_header += 8 - size_with_header % 8;
    }
    void* ref = Allocate(size_with_header);
    if (ref == nullptr) {
      Collect();
      ref = Allocate(size_with_header);
      if (ref == nullptr) {
        throw std::runtime_error("Out of memory");
      }
    }
    Header* header = static_cast<Header*>(ref);
    header->size = size_with_header;
    header->flag = flag;
    header->forward_address_ = nullptr;
    void* body = static_cast<Header*>(ref) + 1;
    return body;
  }

  void* ForwardAddress(void* ref) {
    Header* header = H(ref);
    return static_cast<Header*>(header)->forward_address_;
  }

  void SetForwardAddress(void* ref, void* forward_address) {
    Header* header = H(ref);
    static_cast<Header*>(header)->forward_address_ = forward_address;
  }

 private:
  virtual void* Allocate(size_t size) = 0;
  virtual void Collect() = 0;
};

}  // namespace es

#endif