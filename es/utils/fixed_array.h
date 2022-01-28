#ifndef ES_UTILS_FIXED_ARRAY_H
#define ES_UTILS_FIXED_ARRAY_H

#include <es/gc/heap_object.h>

namespace es {

template<typename T>
class FixedArray : public HeapObject {
 public:
  static Handle<FixedArray<T>> New(std::vector<Handle<T>> elements) {
    std::cout << "FixedArray::New" << std::endl;
    size_t n = elements.size();
    Handle<HeapObject> heap_obj = HeapObject::New(kIntSize + n * kPtrSize);
    SET_VALUE(heap_obj.val(), kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_HANDLE_VALUE(heap_obj.val(), kElementOffset + i * kPtrSize, elements[i], T);
    }
    new (heap_obj.val()) FixedArray<T>();
    return Handle<FixedArray<T>>(heap_obj);
  }

  std::vector<HeapObject**> Pointers() override {
    size_t n = size();
    std::vector<HeapObject**> pointers(n);
    for (size_t i = 0; i < n; i++) {
      pointers[i] = HEAP_PTR(kElementOffset + i * kPtrSize);
    }
    return pointers;
  }

  static Handle<FixedArray<T>> New(size_t n) {
    Handle<HeapObject> heap_obj = HeapObject::New(kIntSize + n * kPtrSize);
    SET_VALUE(heap_obj.val(), kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_HANDLE_VALUE(heap_obj.val(), kElementOffset + i * kPtrSize, Handle<T>(), T);
    }
    return Handle<FixedArray<T>>(new (heap_obj.val()) FixedArray<T>());
  }

  size_t size() { return READ_VALUE(this, kSizeOffset, size_t); }
  Handle<T> Get(size_t i) { return READ_HANDLE_VALUE(this, kElementOffset + i * kPtrSize, T); }
  void Set(size_t i, Handle<T> val) { SET_HANDLE_VALUE(this, kElementOffset + i * kPtrSize, val, T); }

  std::string ToString() override { return "FixedArray(" + std::to_string(size()) + ")"; }

 private:
  static constexpr size_t kSizeOffset = kHeapObjectOffset;
  static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;
};

}  // namespace es

#endif  // ES_UTILS_FIXED_ARRAY_H