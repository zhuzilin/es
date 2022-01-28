#ifndef ES_UTILS_MACROS_H
#define ES_UTILS_MACROS_H

namespace es {

constexpr size_t kIntSize = sizeof(int);
constexpr size_t kSizeTSize = sizeof(size_t);
constexpr size_t kBoolSize = sizeof(bool);
constexpr size_t kDoubleSize = sizeof(double);
constexpr size_t kCharSize = sizeof(char);
constexpr size_t kChar16Size = sizeof(char16_t);
constexpr size_t kPtrSize = sizeof(void*);
constexpr size_t kFuncPtrSize = sizeof(void* (*));

#define PTR(ptr, offset) \
  (reinterpret_cast<char*>(ptr) + offset)

#define TYPED_PTR(ptr, offset, type) \
  reinterpret_cast<type*>((reinterpret_cast<char*>(ptr) + offset))

#define HEAP_PTR(offset) \
  reinterpret_cast<HeapObject**>((reinterpret_cast<char*>(this) + offset))

#define SET_HANDLE_VALUE(ptr, offset, handle, type) \
  *reinterpret_cast<type**>(PTR(ptr, offset)) = handle.val()

#define READ_HANDLE_VALUE(ptr, offset, type) \
  Handle<type>(*reinterpret_cast<type**>(PTR(ptr, offset)))
// Read non pointer value, e.g. bool, Type.
#define READ_VALUE(ptr, offset, type) \
  *reinterpret_cast<type*>(PTR(ptr, offset))
// Set Method
#define SET_VALUE(ptr, offset, val, type) \
  *reinterpret_cast<type*>(PTR(ptr, offset)) = val

enum GCFlag {
  CONST = 1,
  BIG   = 1 << 1,
};

}  // namespace es

#endif  // ES_UTILS_MACROS_H