#ifndef ES_TYPES_SAME_VALUE_H
#define ES_TYPES_SAME_VALUE_H

#include <es/types/base.h>

namespace es {

bool SameValue(JSValue x, JSValue y) {
  if (x.type() != y.type())
    return false;
  switch (x.type()) {
    case Type::JS_UNDEFINED:
      return true;
    case Type::JS_NULL:
      return true;
    case Type::JS_NUMBER: {
      if (number::IsNaN(x) && number::IsNaN(y))
        return true;
      double dx = number::data(x);
      double dy = number::data(y);
      if (dx == dy && dx == 0.0) {
        return signbit(dx) == signbit(dy);
      }
      return dx == dy;
    }
    case Type::JS_LONG_STRING:
    case Type::JS_STRING: {
      return string::data(x) == string::data(y);
    }
    case Type::JS_BOOL: {
      return boolean::data(x) == boolean::data(y);
    }
    default:
      return x == y;
  }
}

}  // namespace es

#endif  // ES_TYPES_SAME_VALUE_H