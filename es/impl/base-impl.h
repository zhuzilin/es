#ifndef ES_IMPL_BASE_IMPL_H
#define ES_IMPL_BASE_IMPL_H

#include <es/types/base.h>
#include <es/error.h>

namespace es {

void CheckObjectCoercible(Handle<Error>& e, Handle<JSValue> val) {
  if (val.val()->IsUndefined() || val.val()->IsNull()) {
    e = Error::TypeError(u"undefined or null is not coercible");
  }
}

bool JSValue::IsCallable() {
  return IsObject() && READ_VALUE(this, JSObject::kIsCallableOffset, bool) || IsGetterSetter();
}

bool JSValue::IsConstructor() {
  return IsObject() && READ_VALUE(this, JSObject::kIsConstructorOffset, bool);
}

}  // namespace es

#endif  // ES_IMPL_BASE_IMPL_H