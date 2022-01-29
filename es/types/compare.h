#ifndef ES_TYPES_COMPARE_H
#define ES_TYPES_COMPARE_H

#include <math.h>

#include <es/types/conversion.h>

namespace es {

// 11.8.5 The Abstract Relational Comparison Algorithm
// x < y
Handle<JSValue> LessThan(Error* e, Handle<JSValue> x, Handle<JSValue> y, bool left_first = true) {
  Handle<JSValue> px, py;
  if (left_first) {
    px = ToPrimitive(e, x, u"Number");
    if (!e->IsOk()) return Undefined::Instance();
    py = ToPrimitive(e, y, u"Number");
    if (!e->IsOk()) return Undefined::Instance();
  } else {
    py = ToPrimitive(e, y, u"Number");
    if (!e->IsOk()) return Undefined::Instance();
    px = ToPrimitive(e, x, u"Number");
    if (!e->IsOk()) return Undefined::Instance();
  }
  if (!(px.val()->IsString() && py.val()->IsString())) {  // 3
    double nx = ToNumber(e, px);
    if (!e->IsOk()) return Undefined::Instance();
    double ny = ToNumber(e, py);
    if (!e->IsOk()) return Undefined::Instance();
    if (isnan(nx) || isnan(ny))
      return Undefined::Instance();
    if (nx == ny)  // this includes +0 vs -0
      return Bool::False();
    if (isinf(nx) && !signbit(nx))  // nx = +inf
      return Bool::False();
    if (isinf(ny) && !signbit(ny))  // ny = +inf
      return Bool::True();
    if (isinf(ny) && signbit(ny))  // ny = -inf
      return Bool::False();
    if (isinf(nx) && signbit(nx))  // nx = -inf
      return Bool::True();
    return Bool::Wrap(nx < ny);
  } else {  // 4
    Handle<String> sx = ToString(e, px);
    if (!e->IsOk()) return Undefined::Instance();
    Handle<String> sy = ToString(e, py);
    if (!e->IsOk()) return Undefined::Instance();
    return Bool::Wrap(*sx.val() < *sy.val());
  }
}

// 11.9.3 The Abstract Equality Comparison Algorithm
// x == y
bool Equal(Error* e, Handle<JSValue> x, Handle<JSValue> y) {
  if (x.val()->type() == y.val()->type()) {
    if (x.val()->IsUndefined()) {
      return true;
    } else if (x.val()->IsNull()) {
      return true;
    } else if (x.val()->IsNumber()) {
      Handle<Number> numx = static_cast<Handle<Number>>(x);
      Handle<Number> numy = static_cast<Handle<Number>>(y);
      if (numx.val()->IsNaN() || numy.val()->IsNaN())
        return false;
      return numx.val()->data() == numy.val()->data();
    } else if (x.val()->IsString()) {
      Handle<String> sx = static_cast<Handle<String>>(x);
      Handle<String> sy = static_cast<Handle<String>>(y);
      return sx.val()->data() == sy.val()->data();
    }
    return x.val() == y.val();
  }
  if (x.val()->IsNull() && y.val()->IsUndefined()) {  // 2
    return true;
  } else if (x.val()->IsUndefined() && y.val()->IsNull()) {  // 3
    return true;
  } else if (x.val()->IsNumber() && y.val()->IsString()) {  // 4
    double numy = ToNumber(e, y);
    if (!e->IsOk()) return false;
    return Equal(e, x, Number::New(numy));
  } else if (x.val()->IsString() && y.val()->IsNumber()) {  // 5
    double numx = ToNumber(e, x);
    if (!e->IsOk()) return false;
    return Equal(e, Number::New(numx), y);
  } else if (x.val()->IsBool()) {  // 6
    double numx = ToNumber(e, x);
    if (!e->IsOk()) return false;
    return Equal(e, Number::New(numx), y);
  } else if (y.val()->IsBool()) {  // 7
    double numy = ToNumber(e, x);
    if (!e->IsOk()) return false;
    return Equal(e, x, Number::New(numy));
  } else if ((x.val()->IsNumber() || x.val()->IsString()) && y.val()->IsObject()) {  // 8
    Handle<JSValue> primy = ToPrimitive(e, y, u"");
    if (!e->IsOk()) return false;
    return Equal(e, x, primy);
  } else if (x.val()->IsObject() && (y.val()->IsNumber() || y.val()->IsString())) {  // 9
    Handle<JSValue> primx = ToPrimitive(e, x, u"");
    if (!e->IsOk()) return false;
    return Equal(e, primx, y);
  }
  return false;
}

// 11.9.6 The Strict Equality Comparison Algorithm
// x === y
bool StrictEqual(Error* e, Handle<JSValue> x, Handle<JSValue> y) {
  if (x.val()->type() != y.val()->type())
    return false;
  switch (x.val()->type()) {
    case JSValue::JS_UNDEFINED:
      return true;
    case JSValue::JS_NULL:
      return true;
    case JSValue::JS_NUMBER: {
      Handle<Number> num_x = static_cast<Handle<Number>>(x);
      Handle<Number> num_y = static_cast<Handle<Number>>(y);
      if (num_x.val()->IsNaN() || num_y.val()->IsNaN())
        return false;
      double dx = num_x.val()->data();
      double dy = num_y.val()->data();
      return dx == dy;
    }
    case JSValue::JS_STRING: {
      Handle<String> str_x = static_cast<Handle<String>>(x);
      Handle<String> str_y = static_cast<Handle<String>>(y);
      return str_x.val()->data() == str_y.val()->data();
    }
    case JSValue::JS_BOOL: {
      Handle<Bool> b_x = static_cast<Handle<Bool>>(x);
      Handle<Bool> b_y = static_cast<Handle<Bool>>(y);
      return b_x.val()->data() == b_y.val()->data();
    }
    default:
      return x.val() == y.val();
  }
}

}  // namespace es

#endif  // ES_TYPES_COMPARE_H