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

}  // namespace es

#endif  // ES_IMPL_BASE_IMPL_H