#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

#include <es/types/type.h>
#include <es/utils/macros.h>
#include <es/gc/handle.h>

namespace es {

void* Allocate(size_t size, flag_t flag);

class HeapObject {
 public:
  static Handle<HeapObject> New(size_t size, flag_t flag = 0) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "HeapObject::New " << size << " " << int(flag) << "\n";
#endif
    Handle<HeapObject> heap_obj(static_cast<HeapObject*>(Allocate(size + kSizeTSize, flag)));
    ASSERT(reinterpret_cast<uint64_t>(heap_obj.val()) % 8 == 0);

    // type value should be init by each variable after their member elements
    // are initialized.
    SET_VALUE(heap_obj.val(), kTypeOffset, JS_UNINIT, Type);
    return heap_obj;
  }

  inline Type type() { return READ_VALUE(this, kTypeOffset, Type); }
  inline void SetType(Type t) { SET_VALUE(this, kTypeOffset, t, Type); }

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;
  void* operator new(size_t, void* ptr) = delete;

  static std::vector<HeapObject**> Pointers(HeapObject* heap_obj);

  static std::string ToString(Type type);

 public:
  static constexpr size_t kTypeOffset = 0;
  static constexpr size_t kHeapObjectOffset = kTypeOffset + kSizeTSize;
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H