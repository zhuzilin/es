#ifndef ES_TYPES_BUILTIN_MATH_OBJECT
#define ES_TYPES_BUILTIN_MATH_OBJECT

#include <es/types/object.h>

namespace es {

double ToNumber(Handle<Error>& e, Handle<JSValue> input);

class Math : public JSObject {
 public:
  static Handle<Math> Instance() {
  static Handle<Math> singleton = Math::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"Math");
  }

  // 15.8.2.1 abs (x)
  static Handle<JSValue> abs(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return Number::Zero();
    if (isinf(num))
      return Number::Infinity();
    return Number::New(::fabs(num));
  }

  // 15.8.2.6 ceil (x)
  static Handle<JSValue> ceil(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return signbit(num) ? Number::NegativeZero() : Number::Zero();
    if (isinf(num))
      return signbit(num) ? Number::NegativeInfinity() : Number::Infinity();
    return Number::New(::ceil(num));
  }

  // 15.8.2.7 cos (x)
  static Handle<JSValue> cos(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return Number::One();
    if (isinf(num))
      return signbit(num) ? Number::Zero() : Number::Infinity();
    return Number::New(::cos(num));
  }

  // 15.8.2.8 exp (x)
  static Handle<JSValue> exp(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return Number::One();
    if (isinf(num))
      return signbit(num) ? Number::Infinity() : Number::Zero();
    return Number::New(::exp(num));
  }

  // 15.8.2.9 floor (x)
  static Handle<JSValue> floor(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return signbit(num) ? Number::NegativeZero() : Number::Zero();
    if (isinf(num))
      return signbit(num) ? Number::NegativeInfinity() : Number::Infinity();
    return Number::New(::floor(num));
  }

  // 15.8.2.11 max ( [ value1 [ , value2 [ , … ] ] ] )
  static Handle<JSValue> max(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NegativeInfinity();
    double value1 = ToNumber(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    double value2 = nan("");
    if (likely(vals.size() >= 2)) {
      value2 = ToNumber(e, vals[1]);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    if (isnan(value1) || isnan(value2))
      return Number::NaN();
    if (value1 == 0 && value2 == 0 && (!signbit(value1) || !signbit(value2)))
      return Number::Zero();
    else if (value1 >= value2)
      return Number::New(value1);
    else
      return Number::New(value2);
  }

  // 15.8.2.13 pow (x, y)
  static Handle<JSValue> pow(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 2)
      return Number::NaN();
    double x = ToNumber(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    double y = ToNumber(e, vals[1]);
    if (isnan(y))
      return Number::NaN();
    if (y == 0)
      return Number::One();
    if (isnan(x))
      return Number::NaN();
    return Number::New(::pow(x, y));
  }

  // 15.8.2.15 round (x)
  static Handle<JSValue> round(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return signbit(num) ? Number::NegativeZero() : Number::Zero();
    if (isinf(num))
      return signbit(num) ? Number::NegativeInfinity() : Number::Infinity();
    return Number::New(::round(num));
  }

  // 15.8.2.16 sin (x)
  static Handle<JSValue> sin(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return signbit(num) ? Number::NegativeZero() : Number::Zero();
    if (isinf(num))
      return Number::NaN();
    return Number::New(::sin(num));
  }

  // 15.8.2.17 sqrt (x)
  static Handle<JSValue> sqrt(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    double num = ToNumber(e, vals[0]);
    if (isnan(num))
      return Number::NaN();
    if (num == 0)
      return signbit(num) ? Number::NegativeZero() : Number::Zero();
    if (signbit(num))
      return Number::NaN();
    if (isinf(num))
      return Number::Infinity();
    return Number::New(::sqrt(num));
  }

 private:
  static Handle<Math> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Math", true, Handle<JSValue>(), false, false, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<Math>(jsobj);
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_MATH_OBJECT