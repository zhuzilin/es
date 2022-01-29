#ifndef ES_TYPES_BUILTIN_MATH_OBJECT
#define ES_TYPES_BUILTIN_MATH_OBJECT

#include <es/types/object.h>

namespace es {

double ToNumber(Error* e, Handle<JSValue> input);

class Math : public JSObject {
 public:
  static Handle<Math> Instance() {
  static Handle<Math> singleton = Math::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"Math");
  }

  static Handle<JSValue> max(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NegativeInfinity();
    double value1 = ToNumber(e, vals[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    double value2;
    if (vals.size() < 2) {
      value2 = ToNumber(e, vals[1]);
      if (!e->IsOk()) return Handle<JSValue>();
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

 private:
  static Handle<Math> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_FUNC, u"Math", true, Handle<JSValue>(), false, false, nullptr, 0, flag);

    return Handle<Math>(new (jsobj.val()) Math());
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_MATH_OBJECT