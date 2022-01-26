#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

#include <es/utils/macros.h>

namespace es {

class HeapObject {
 public:
  virtual std::vector<void*> Pointers() = 0;

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;

  void* operator new(size_t, void* ptr) {
    return ptr;
  }

  static HeapObject* New(size_t size) {
    if (size % 4 != 0) {
      size = size + 4 - size % 4;
    }
    return static_cast<HeapObject*>(Alloc(size + kPtrSize));
  }

  static void* Alloc(size_t size) {
    if (offset + size + kPtrSize > kMemSize) {
      mem = malloc(HeapObject::kMemSize);
      offset = 0;
    }
    void* ptr = TYPED_PTR(mem, offset, void*);
    offset += size + kPtrSize;
    return ptr;
  }

 protected:
  static constexpr size_t kVPtrOffset = 0;
  static constexpr size_t kHeapObjectOffset = kVPtrOffset + kPtrSize;

  static size_t offset;
  static void* mem;
  static constexpr size_t kMemSize = 1 * 1024 * 1024 * 1024;
};

size_t HeapObject::offset = 0;
void* HeapObject::mem = malloc(HeapObject::kMemSize);

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H