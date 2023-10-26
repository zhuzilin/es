#ifndef ES_TYPES_COMPARE_H
#define ES_TYPES_COMPARE_H

#include <math.h>

#include <es/types/conversion.h>

namespace es {

// 11.8.5 The Abstract Relational Comparison Algorithm
// x < y
JSValue LessThan(JSValue& e, JSValue x, JSValue y, bool left_first = true) {
  JSValue px, py;
  if (left_first) {
    px = ToPrimitive(e, x, u"Number");
    if (unlikely(!error::IsOk(e))) return undefined::New();
    py = ToPrimitive(e, y, u"Number");
    if (unlikely(!error::IsOk(e))) return undefined::New();
  } else {
    py = ToPrimitive(e, y, u"Number");
    if (unlikely(!error::IsOk(e))) return undefined::New();
    px = ToPrimitive(e, x, u"Number");
    if (unlikely(!error::IsOk(e))) return undefined::New();
  }
  if (!(px.IsString() && py.IsString())) {  // 3
    double nx = ToNumber(e, px);
    if (unlikely(!error::IsOk(e))) return undefined::New();
    double ny = ToNumber(e, py);
    if (unlikely(!error::IsOk(e))) return undefined::New();
    if (isnan(nx) || isnan(ny))
      return undefined::New();
    if (nx == ny)  // this includes +0 vs -0
      return boolean::False();
    if (isinf(nx) && !signbit(nx))  // nx = +inf
      return boolean::False();
    if (isinf(ny) && !signbit(ny))  // ny = +inf
      return boolean::True();
    if (isinf(ny) && signbit(ny))  // ny = -inf
      return boolean::False();
    if (isinf(nx) && signbit(nx))  // nx = -inf
      return boolean::True();
    return boolean::New(nx < ny);
  } else {  // 4
    JSValue sx = ToString(e, px);
    if (unlikely(!error::IsOk(e))) return undefined::New();
    JSValue sy = ToString(e, py);
    if (unlikely(!error::IsOk(e))) return undefined::New();
    return boolean::New(string::data(sx) < string::data(sy));
  }
}

// 11.9.3 The Abstract Equality Comparison Algorithm
// x == y
bool Equal(JSValue& e, JSValue x, JSValue y) {
  if (x.type() == y.type()) {
    if (x.IsUndefined()) {
      return true;
    } else if (x.IsNull()) {
      return true;
    } else if (x.IsNumber()) {
      if (number::IsNaN(x) || number::IsNaN(y))
        return false;
      if (number::IsInfinity(x) || number::IsInfinity(y)) {
        return number::data(x) == number::data(y);
      }
      return ApproximatelyEqual(number::data(x), number::data(y));
    } else if (x.IsString()) {
      return string::data(x) == string::data(y);
    }
    return x.handle().val() == y.handle().val();
  }
  if (x.IsNull() && y.IsUndefined()) {  // 2
    return true;
  } else if (x.IsUndefined() && y.IsNull()) {  // 3
    return true;
  } else if (x.IsNumber() && y.IsString()) {  // 4
    double numy = ToNumber(e, y);
    if (unlikely(!error::IsOk(e))) return false;
    return Equal(e, x, number::New(numy));
  } else if (x.IsString() && y.IsNumber()) {  // 5
    double numx = ToNumber(e, x);
    if (unlikely(!error::IsOk(e))) return false;
    return Equal(e, number::New(numx), y);
  } else if (x.IsBool()) {  // 6
    double numx = ToNumber(e, x);
    if (unlikely(!error::IsOk(e))) return false;
    return Equal(e, number::New(numx), y);
  } else if (y.IsBool()) {  // 7
    double numy = ToNumber(e, x);
    if (unlikely(!error::IsOk(e))) return false;
    return Equal(e, x, number::New(numy));
  } else if ((x.IsNumber() || x.IsString()) && y.IsObject()) {  // 8
    JSValue primy = ToPrimitive(e, y, u"");
    if (unlikely(!error::IsOk(e))) return false;
    return Equal(e, x, primy);
  } else if (x.IsObject() && (y.IsNumber() || y.IsString())) {  // 9
    JSValue primx = ToPrimitive(e, x, u"");
    if (unlikely(!error::IsOk(e))) return false;
    return Equal(e, primx, y);
  }
  return false;
}

// 11.9.6 The Strict Equality Comparison Algorithm
// x === y
bool StrictEqual(JSValue& e, JSValue x, JSValue y) {
  if (x.type() != y.type())
    return false;
  switch (x.type()) {
    case Type::JS_UNDEFINED:
      return true;
    case Type::JS_NULL:
      return true;
    case Type::JS_NUMBER: {
      if (number::IsNaN(x) || number::IsNaN(y))
        return false;
      if (number::IsInfinity(x) || number::IsInfinity(y)) {
        return number::data(x) == number::data(y);
      }
      double dx = number::data(x);
      double dy = number::data(y);
      return ApproximatelyEqual(dx, dy);
    }
    case Type::JS_LONG_STRING:
    case Type::JS_STRING: {
      return string::data(x) == string::data(y);
    }
    case Type::JS_BOOL: {
      return boolean::data(x) == boolean::data(y);
    }
    default:
      return x.handle().val() == y.handle().val();
  }
}

}  // namespace es

#endif  // ES_TYPES_COMPARE_H