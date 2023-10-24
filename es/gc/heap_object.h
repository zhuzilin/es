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
    Handle<HeapObject> heap_obj(static_cast<HeapObject*>(Allocate(size, flag)));
    //std::cout << "HeapObject::New " << heap_obj.val() << " handle: " << heap_obj.ptr() << std::endl;
    return heap_obj;
  }

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;
  void* operator new(size_t, void* ptr) = delete;

 public:
  static constexpr size_t kHeapObjectOffset = 0;
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H