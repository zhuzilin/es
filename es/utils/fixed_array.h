#ifndef ES_UTILS_FIXED_ARRAY_H
#define ES_UTILS_FIXED_ARRAY_H

#include <es/gc/heap_object.h>

namespace es {

template<typename T>
class FixedArray : public HeapObject {
 public:
  static FixedArray<T>* New(std::vector<T*> elements) {
    std::cout << "FixedArray::New" << std::endl;
    size_t n = elements.size();
    HeapObject* heap_obj = HeapObject::New(kIntSize + n * kPtrSize);
    SET_VALUE(heap_obj, kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_VALUE(heap_obj, kElementOffset + i * kPtrSize, elements[i], T*);
    }
    return new (heap_obj) FixedArray<T>();
  }

  std::vector<void*> Pointers() override {
    size_t n = size();
    std::vector<void*> pointers(n);
    for (size_t i = 0; i < n; i++) {
      pointers[i] = HEAP_PTR(kElementOffset + i * kPtrSize);
    }
    return pointers;
  }

  static FixedArray<T>* New(size_t n) {
    HeapObject* heap_obj = HeapObject::New(kIntSize + n * kPtrSize);
    SET_VALUE(heap_obj, kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_VALUE(heap_obj, kElementOffset + i * kPtrSize, nullptr, T*);
    }
    return new (heap_obj) FixedArray<T>();
  }

  size_t size() { return READ_VALUE(this, kSizeOffset, size_t); }
  T* Get(size_t i) { return READ_VALUE(this, kElementOffset + i * kPtrSize, T*); }
  void Set(size_t i, T* val) { SET_VALUE(this, kElementOffset + i * kPtrSize, val, T*); }

  std::string ToString() override { return "FixedArray(" + std::to_string(size()) + ")"; }

 private:
  static constexpr size_t kSizeOffset = kHeapObjectOffset;
  static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;
};

}  // namespace es

#endif  // ES_UTILS_FIXED_ARRAY_H