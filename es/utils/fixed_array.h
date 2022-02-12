#ifndef ES_UHeapObjectILS_FIXED_ARRAY_H
#define ES_UHeapObjectILS_FIXED_ARRAY_H

#include <es/gc/heap_object.h>

namespace es {

class FixedArray : public HeapObject {
 public:
  template<typename T>
  static Handle<FixedArray> New(std::vector<Handle<T>> elements) {
    size_t n = elements.size();
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "FixedArray::New " << n << "\n";
#endif
    Handle<HeapObject> heap_obj = HeapObject::New(kSizeTSize + n * kPtrSize);

    SET_VALUE(heap_obj.val(), kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_HANDLE_VALUE(heap_obj.val(), kElementOffset + i * kPtrSize, elements[i], HeapObject);
    }
    heap_obj.val()->SetType(FIXED_ARRAY);
    return Handle<FixedArray>(heap_obj);
  }

  size_t size() { return READ_VALUE(this, kSizeOffset, size_t); }
  Handle<HeapObject> Get(size_t i) { return READ_HANDLE_VALUE(this, kElementOffset + i * kPtrSize, HeapObject); }
  HeapObject* GetRaw(size_t i) { return READ_VALUE(this, kElementOffset + i * kPtrSize, HeapObject*); }
  void Set(size_t i, Handle<HeapObject> val) { SET_HANDLE_VALUE(this, kElementOffset + i * kPtrSize, val, HeapObject); }

 public:
  static constexpr size_t kSizeOffset = kHeapObjectOffset;
  static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;
};

}  // namespace es

#endif  // ES_UHeapObjectILS_FIXED_ARRAY_H