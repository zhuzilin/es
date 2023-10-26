#ifndef ES_UTILS_MACROS_H
#define ES_UTILS_MACROS_H

#include <stdlib.h>

namespace es {

constexpr size_t kIntSize = sizeof(int);
constexpr size_t kSizeTSize = sizeof(size_t);
constexpr size_t kBoolSize = sizeof(bool);
constexpr size_t kDoubleSize = sizeof(double);
constexpr size_t kCharSize = sizeof(char);
constexpr size_t kChar16Size = sizeof(char16_t);
constexpr size_t kFuncPtrSize = sizeof(void* (*));

#define PTR(ptr, offset) \
  (reinterpret_cast<char*>(ptr) + offset)

#define TYPED_PTR(ptr, offset, type) \
  reinterpret_cast<type*>((reinterpret_cast<char*>(ptr) + offset))

#define HEAP_PTR(ptr, offset) \
  reinterpret_cast<HeapObject**>(PTR(ptr, offset))

#define SET_JSVALUE(ptr, offset, jsval) \
  *reinterpret_cast<JSValue*>(PTR(ptr, offset)) = jsval
#define GET_JSVALUE(ptr, offset) \
  *reinterpret_cast<JSValue*>(PTR(ptr, offset))

// Read non pointer value, e.g. bool, Type.
#define READ_VALUE(ptr, offset, type) \
  *reinterpret_cast<type*>(PTR(ptr, offset))
// Set Method
#define SET_VALUE(ptr, offset, val, type) \
  *reinterpret_cast<type*>(PTR(ptr, offset)) = val

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifdef TEST
#define ASSERT(x) assert(x)
#define TEST_LOG(x...) \
  if (unlikely(log::Debugger::On())) \
    log::PrintSource(x)
#else
#define ASSERT(x)
#define TEST_LOG(x...)
#endif

}  // namespace es

#endif  // ES_UTILS_MACROS_H