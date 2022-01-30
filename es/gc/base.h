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
    header->forward_address = nullptr;
    void* body = static_cast<Header*>(ref) + 1;
    assert(ForwardAddress(body) == nullptr);
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
  virtual void* Allocate(size_t size) = 0;
  virtual void Collect() = 0;
};

}  // namespace es

#endif