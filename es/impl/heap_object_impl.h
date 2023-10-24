#ifndef ES_IMPL_HEAP_OBJECT_IMPL_H
#define ES_IMPL_HEAP_OBJECT_IMPL_H

#include <es/types.h>

namespace es {

template<typename T>
std::string Handle<T>::ToString() {
  if (IsNullptr())
    return "nullptr";
  return JSValue::ToString(val());
}

}  // namespace es

#endif  // ES_IMPL_HEAP_OBJECT_IMPL_H
