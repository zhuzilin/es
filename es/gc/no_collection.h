#ifndef ES_GC_NO_COLLECTION
#define ES_GC_NO_COLLECTION

#include <stdlib.h>

#include <es/gc/base_collection.h>
#include <es/utils/helper.h>
#include <es/utils/macros.h>

namespace es {

struct NoCollection : public GC<NoCollection> {
  NoCollection(size_t segment_size) :
    segment_size_(segment_size), memsize_(0), offset_(0) {}

  void* AllocateImpl(size_t size, flag_t flag) {
    if (offset_ + size > memsize_) {
      return nullptr;
    }
    void* ptr = TYPED_PTR(mem_, offset_, void*);
    offset_ += size;
    // Set header
    Header* header = reinterpret_cast<Header*>(ptr);
    header->size = size;
    header->flag = flag;

    return ptr;
  }

  void CollectImpl() {
    // Simply reallocate a segment.
    mem_ = malloc(segment_size_);
    memsize_ = segment_size_;
    offset_ = 0;
  }

  const size_t segment_size_;
  void* mem_;
  size_t memsize_ = 0;
  size_t offset_ = 0;
};

}  // namespace es

#endif  // ES_GC_NO_COLLECTION
