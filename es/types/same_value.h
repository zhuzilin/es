#ifndef ES_TYPES_SAME_VALUE_H
#define ES_TYPES_SAME_VALUE_H

#include <es/types/base.h>

namespace es {

bool SameValue(Handle<JSValue> x, Handle<JSValue> y) {
  if (x.val()->type() != y.val()->type())
    return false;
  switch (x.val()->type()) {
    case Type::JS_UNDEFINED:
      return true;
    case Type::JS_NULL:
      return true;
    case Type::JS_NUMBER: {
      Handle<Number> num_x = static_cast<Handle<Number>>(x);
      Handle<Number> num_y = static_cast<Handle<Number>>(y);
      if (num_x.val()->IsNaN() && num_y.val()->IsNaN())
        return true;
      double dx = num_x.val()->data();
      double dy = num_y.val()->data();
      if (dx == dy && dx == 0.0) {
        return signbit(dx) == signbit(dy);
      }
      return dx == dy;
    }
    case Type::JS_LONG_STRING:
    case Type::JS_STRING: {
      Handle<String> str_x = static_cast<Handle<String>>(x);
      Handle<String> str_y = static_cast<Handle<String>>(y);
      return StringEqual(str_x, str_y);
    }
    case Type::JS_BOOL: {
      Handle<Bool> b_x = static_cast<Handle<Bool>>(x);
      Handle<Bool> b_y = static_cast<Handle<Bool>>(y);
      return b_x.val()->data() == b_y.val()->data();
    }
    default:
      return x.val() == y.val();
  }
}

}  // namespace es

#endif  // ES_TYPES_SAME_VALUE_H