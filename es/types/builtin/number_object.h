#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

double ToNumber(JSValue& e, JSValue input);
double ToInteger(JSValue& e, JSValue input);
JSValue ToObject(JSValue& e, JSValue input);
JSValue NumberToString(double m);

namespace number_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Number", true, number::Zero(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = number_proto::New(GCFlag::SINGLE);
  return singleton;
}

// 15.7.4.2 Number.prototype.toString ( [ radix ] )
inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (!val.IsNumber() && !val.IsNumberObject()) {
    e = error::TypeError(u"Number.prototype.toString called by non-number");
    return JSValue();
  }
  double num = ToNumber(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int radix = 10;
  if (vals.size() > 0 && !vals[0].IsUndefined()) {
    radix = ToInteger(e, vals[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (radix < 2 || radix > 36) {
      e = error::RangeError(u"Number.prototype.toString radix not in [2, 36]");
      return JSValue();
    }
  }
  // TODO(zhuzilin) support other radix
  ASSERT(radix == 10);
  return NumberToString(num);
}

inline JSValue toLocaleString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue valueOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (val.IsNumberObject()) {
    return js_object::PrimitiveValue(val);
  } else if (val.IsNumber()) {
    return val;
  }
  e = error::TypeError(u"Number.prototype.valueOf called with non-number");
  return JSValue();
}

inline JSValue toFixed(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toExponential(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toPrecision(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace number_proto

namespace number_object {

inline JSValue New(JSValue primitive_value) {
  JSValue jsobj = js_object::New(
    u"Number", true, primitive_value, false, false, nullptr, 0
  );

  jsobj.SetType(OBJ_NUMBER);
  js_object::SetPrototype(jsobj, number_proto::Instance());
  return jsobj;
}

}  // namespace number_object

namespace number_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Number", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_NUMBER_CONSTRUCTOR);
  return JSValue(jsobj);
}

inline JSValue Instance() {
  static JSValue singleton = number_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Number() { [native code] }");
}

}  // namespace number_constructor

JSValue Call__NumberConstructor(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Construct__NumberConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT