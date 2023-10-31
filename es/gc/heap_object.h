#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

#include <es/types/type.h>
#include <es/utils/macros.h>
#include <es/gc/handle.h>

namespace es {

class HeapObject {
 public:
  template<flag_t flag = 0>
  static Handle<HeapObject> New(size_t size);

  template<uint32_t size, flag_t flag = 0>
  static Handle<HeapObject> New();

  inline Type type() { return h_.type; }
  inline void SetType(Type t) { h_.type = t; }

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;
  void* operator new(size_t, void* ptr) = delete;

  static std::vector<HeapObject**> Pointers(HeapObject* heap_obj);

  static std::string ToString(Type type);

 public:
  static_assert(sizeof(Header) == 16);
  static constexpr size_t kHeaderOffset = sizeof(Header);
  static constexpr size_t kHeapObjectOffset = kHeaderOffset;
  Header h_;
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H