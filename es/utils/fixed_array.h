#ifndef ES_UHeapObjectILS_FIXED_ARRAY_H
#define ES_UHeapObjectILS_FIXED_ARRAY_H

#include <es/gc/heap_object.h>

namespace es {

namespace fixed_array {

constexpr size_t kSizeOffset = 0;
constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;

inline JSValue New(std::vector<JSValue> elements) {
  JSValue jsval;
  size_t n = elements.size();
  std::cout << "enter fa" << std::endl;
  jsval.handle() = HeapObject::New(kSizeTSize + n * sizeof(JSValue));

  SET_VALUE(jsval.handle().val(), kSizeOffset, n, size_t);
  for (size_t i = 0; i < n; i++) {
    SET_JSVALUE(jsval.handle().val(), kElementOffset + i * sizeof(JSValue), elements[i]);
  }
  jsval.SetType(FIXED_ARRAY);
  return jsval;
}

inline size_t size(JSValue arr) { return READ_VALUE(arr.handle().val(), kSizeOffset, size_t); }
inline JSValue Get(JSValue arr, size_t i) { return GET_JSVALUE(arr.handle().val(), kElementOffset + i * sizeof(JSValue)); }
inline void Set(JSValue& arr, size_t i, JSValue val) { SET_JSVALUE(arr.handle().val(), kElementOffset + i * sizeof(JSValue), val); }

}  // namespace fixed_array
}  // namespace es

#endif  // ES_UHeapObjectILS_FIXED_ARRAY_H