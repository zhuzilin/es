#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

#include <es/utils/macros.h>
#include <es/gc/handle.h>

namespace es {

void* Allocate(size_t size, flag_t flag);

class HeapObject {
 public:
  static Handle<HeapObject> New(size_t size, flag_t flag = 0) {
    return Handle<HeapObject>(static_cast<HeapObject*>(Allocate(size + kPtrSize, flag)));
  }

  virtual std::vector<HeapObject**> Pointers() = 0;

  virtual inline bool IsJSValue() { return false; }

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;

  void* operator new(size_t, void* ptr) {
    return ptr;
  }

  virtual std::string ToString() = 0;

 protected:
  static constexpr size_t kVPtrOffset = 0;
  static constexpr size_t kHeapObjectOffset = kVPtrOffset + kPtrSize;
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H