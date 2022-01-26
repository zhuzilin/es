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
    return static_cast<HeapObject*>(malloc(size + kPtrSize));
  }

 protected:
  static constexpr size_t kVPtrOffset = 0;
  static constexpr size_t kHeapObjectOffset = kVPtrOffset + kPtrSize;
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H