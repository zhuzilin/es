#ifndef ES_TYPES_BUILTIN_MATH_OBJECT
#define ES_TYPES_BUILTIN_MATH_OBJECT

#include <es/types/object.h>

namespace es {

double ToNumber(JSValue& e, JSValue input);

namespace math {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Math", true, JSValue(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = math::New(GCFlag::SINGLE);
  return singleton;
}


inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"Math");
}

// 15.8.2.1 abs (x)
inline JSValue abs(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return number::Zero();
  if (isinf(num))
    return number::Infinity();
  return number::New(::fabs(num));
}

// 15.8.2.6 ceil (x)
inline JSValue ceil(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return signbit(num) ? number::NegativeZero() : number::Zero();
  if (isinf(num))
    return signbit(num) ? number::NegativeInfinity() : number::Infinity();
  return number::New(::ceil(num));
}

// 15.8.2.7 cos (x)
inline JSValue cos(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return number::One();
  if (isinf(num))
    return signbit(num) ? number::Zero() : number::Infinity();
  return number::New(::cos(num));
}

// 15.8.2.8 exp (x)
inline JSValue exp(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return number::One();
  if (isinf(num))
    return signbit(num) ? number::Infinity() : number::Zero();
  return number::New(::exp(num));
}

// 15.8.2.9 floor (x)
inline JSValue floor(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return signbit(num) ? number::NegativeZero() : number::Zero();
  if (isinf(num))
    return signbit(num) ? number::NegativeInfinity() : number::Infinity();
  return number::New(::floor(num));
}

// 15.8.2.11 max ( [ value1 [ , value2 [ , … ] ] ] )
inline JSValue max(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NegativeInfinity();
  double value1 = ToNumber(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  double value2 = nan("");
  if (likely(vals.size() >= 2)) {
    value2 = ToNumber(e, vals[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  if (isnan(value1) || isnan(value2))
    return number::NaN();
  if (value1 == 0 && value2 == 0 && (!signbit(value1) || !signbit(value2)))
    return number::Zero();
  else if (value1 >= value2)
    return number::New(value1);
  else
    return number::New(value2);
}

// 15.8.2.13 pow (x, y)
inline JSValue pow(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 2)
    return number::NaN();
  double x = ToNumber(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  double y = ToNumber(e, vals[1]);
  if (isnan(y))
    return number::NaN();
  if (y == 0)
    return number::One();
  if (isnan(x))
    return number::NaN();
  return number::New(::pow(x, y));
}

// 15.8.2.15 round (x)
inline JSValue round(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return signbit(num) ? number::NegativeZero() : number::Zero();
  if (isinf(num))
    return signbit(num) ? number::NegativeInfinity() : number::Infinity();
  return number::New(::round(num));
}

// 15.8.2.16 sin (x)
inline JSValue sin(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return signbit(num) ? number::NegativeZero() : number::Zero();
  if (isinf(num))
    return number::NaN();
  return number::New(::sin(num));
}

// 15.8.2.17 sqrt (x)
inline JSValue sqrt(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  double num = ToNumber(e, vals[0]);
  if (isnan(num))
    return number::NaN();
  if (num == 0)
    return signbit(num) ? number::NegativeZero() : number::Zero();
  if (signbit(num))
    return number::NaN();
  if (isinf(num))
    return number::Infinity();
  return number::New(::sqrt(num));
}

}  // namespace math
}  // namespace es

#endif  // ES_TYPES_BUILTIN_MATH_OBJECT