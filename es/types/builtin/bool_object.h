#ifndef ES_TYPES_BUILTIN_BOOL_OBJECT
#define ES_TYPES_BUILTIN_BOOL_OBJECT

#include <es/types/object.h>

namespace es {

bool ToBoolean(JSValue);

namespace bool_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Boolean", true, boolean::False(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue& Instance() {
  static JSValue singleton = New(GCFlag::SINGLE);
  return singleton;
}

// 15.6.4.2 Boolean.prototype.toString ( )
inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (val.IsBool()) {
    return ToBoolean(val) ? string::True() : string::False();
  } else if (val.IsBoolObject()) {
    return ToBoolean(js_object::PrimitiveValue(val)) ?
      string::True() : string::False();
  } else {
    e = error::TypeError(u"Boolean.prototype.toString called on non-boolean");
    return JSValue();
  }
}

// 15.6.4.3 Boolean.prototype.valueOf ( )
inline JSValue valueOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (val.IsBool()) {
    return val;
  } else if (val.IsBoolObject()) {
    return js_object::PrimitiveValue(val);
  } else {
    e = error::TypeError(u"Boolean.prototype.valueOf called on non-boolean");
    return JSValue();
  }
}

}  // namespace bool_proto

namespace bool_object {

inline JSValue New(JSValue primitive_value) {
  JSValue jsobj = js_object::New(
    u"Boolean", true, primitive_value, false, false, nullptr, 0
  );

  js_object::SetPrototype(jsobj, bool_proto::Instance());
  jsobj.SetType(OBJ_BOOL);
  return jsobj;
}

}  // namespace bool_object

namespace bool_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Boolean", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_BOOL_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = bool_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Bool() { [native code] }");
}

}

JSValue Call__BoolConstructor(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Construct__BoolConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_BOOL_OBJECT