#ifndef ES_GC_NO_COLLECTION
#define ES_GC_NO_COLLECTION

#include <stdlib.h>

#include <es/gc/base.h>
#include <es/utils/helper.h>
#include <es/utils/macros.h>

namespace es {

class NoCollection : public GC {
 public:
  NoCollection(size_t segment_size) :
    segment_size_(segment_size), memsize_(0), offset_(0) {}

 private:
  void* Allocate(size_t size) override {
    if (offset_ + size + kPtrSize > memsize_) {
      return nullptr;
    }
    void* ptr = TYPED_PTR(mem_, offset_, void*);
    offset_ += size + kPtrSize;  
    return ptr;
  }

  void Collect() override {
    mem_ = malloc(segment_size_);
    memsize_ = segment_size_;
    offset_ = 0;
  }

  void* mem_;
  size_t memsize_ = 0;
  size_t offset_ = 0;
  const size_t segment_size_;
};

}  // namespace es

#endif  // ES_GC_NO_COLLECTION
