#ifndef ES_TYPES_SAME_VALUE_H
#define ES_TYPES_SAME_VALUE_H

#include <es/types/base.h>

namespace es {

bool SameValue(JSValue* x, JSValue* y) {
  if (x->type() != y->type())
    return false;
  switch (x->type()) {
    case JSValue::JS_UNDEFINED:
      return true;
    case JSValue::JS_NULL:
      return true;
    case JSValue::JS_NUMBER: {
      // TODO(zhuzilin) +0 and -0 should be different,
      // figure out how to represent them.
      Number* num_x = static_cast<Number*>(x);
      Number* num_y = static_cast<Number*>(y);
      if (num_x->IsNaN() && num_y->IsNaN())
        return true;
      double dx = num_x->data();
      double dy = num_y->data();
      if (dx == dy && dx == 0.0) {
        return signbit(dx) == signbit(dy);
      }
      return dx == dy;
    }
    case JSValue::JS_STRING: {
      String* str_x = static_cast<String*>(x);
      String* str_y = static_cast<String*>(y);
      return str_x->data() == str_y->data();
    }
    case JSValue::JS_BOOL: {
      Bool* b_x = static_cast<Bool*>(x);
      Bool* b_y = static_cast<Bool*>(y);
      return b_x->data() == b_y->data();
    }
    default:
      return x == y;
  }
}

}  // namespace es

#endif  // ES_TYPES_SAME_VALUE_H