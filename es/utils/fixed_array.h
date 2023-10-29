#ifndef ES_UHeapObjectILS_FIXED_ARRAY_H
#define ES_UHeapObjectILS_FIXED_ARRAY_H

#include <es/gc/heap_object.h>

namespace es {

class FixedArray : public JSValue {
 public:
  template<typename T>
  static Handle<FixedArray> New(std::vector<Handle<T>> elements) {
    size_t n = elements.size();
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "FixedArray::New " << n << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New(kSizeTSize + n * kPtrSize);

    SET_VALUE(jsval.val(), kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_HANDLE_VALUE(jsval.val(), kElementOffset + i * kPtrSize, elements[i], JSValue);
    }
    jsval.val()->SetType(FIXED_ARRAY);
    return Handle<FixedArray>(jsval);
  }

  static Handle<FixedArray> New(uint32_t n) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "FixedArray::New " << n << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New(kSizeTSize + n * kPtrSize);

    SET_VALUE(jsval.val(), kSizeOffset, n, size_t);
    for (size_t i = 0; i < n; i++) {
      SET_VALUE(jsval.val(), kElementOffset + i * kPtrSize, nullptr, JSValue*);
    }

    SET_VALUE(jsval.val(), kSizeOffset, n, size_t);
    jsval.val()->SetType(FIXED_ARRAY);
    return Handle<FixedArray>(jsval);
  }

  size_t size() { return READ_VALUE(this, kSizeOffset, size_t); }
  Handle<JSValue> Get(size_t i) { return READ_HANDLE_VALUE(this, kElementOffset + i * kPtrSize, JSValue); }
  JSValue* GetRaw(size_t i) { return READ_VALUE(this, kElementOffset + i * kPtrSize, JSValue*); }
  void Set(size_t i, Handle<JSValue> val) { SET_HANDLE_VALUE(this, kElementOffset + i * kPtrSize, val, JSValue); }

 public:
  static constexpr size_t kSizeOffset = HeapObject::kHeapObjectOffset;
  static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;
};

}  // namespace es

#endif  // ES_UHeapObjectILS_FIXED_ARRAY_H