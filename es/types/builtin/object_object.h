#ifndef ES_TYPES_BUILTIN_OBJECT_OBJECT
#define ES_TYPES_BUILTIN_OBJECT_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

JSValue ToString(JSValue& e, JSValue input);
JSValue FromPropertyDescriptor(JSValue value);
JSValue ToPropertyDescriptor(JSValue& e, JSValue obj);
JSValue ToObject(JSValue& e, JSValue input);

namespace object_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Object", true, JSValue(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OBJECT);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = object_proto::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (val.IsUndefined())
    return string::New(u"[object Undefined]");
  if (val.IsNull())
    return string::New(u"[object Null]");
  JSValue obj = ToObject(e, val);
  return string::New(u"[object " + js_object::Class(obj) + u"]");
}

inline JSValue toLocaleString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue valueOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  JSValue O = ToObject(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  // TODO(zhuzilin) Host object
  return O;
}

inline JSValue hasOwnProperty(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue P;
  if (vals.size() == 0)
    P = ::es::ToString(e, undefined::New());
  else
    P = ::es::ToString(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();

  JSValue val = Runtime::TopValue();
  JSValue O = ToObject(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();

  JSValue desc = GetOwnProperty(O, P);
  return boolean::New(!desc.IsUndefined());
}

inline JSValue isPrototypeOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (unlikely(vals.size() == 0 || !vals[0].IsObject()))
    return boolean::False();
  JSValue val = Runtime::TopValue();
  JSValue O = ToObject(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue V = js_object::Prototype(vals[0]);
  while (!V.IsNull()) {
    if (V == O)
      return boolean::True();
    V = js_object::Prototype(V);
  }
  return boolean::False();
}

inline JSValue propertyIsEnumerable(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace object_proto

namespace object_object {

inline JSValue New() {
  JSValue jsobj = js_object::New(
    u"Object", true, JSValue(), false, false, nullptr, 0
  );

  jsobj.SetType(OBJ_OBJECT);
  js_object::SetPrototype(jsobj, object_proto::Instance());
  return jsobj;
}

}  // namespace object_object

namespace object_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Object", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_OBJECT_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = object_constructor::New(GCFlag::SINGLE);
  return singleton;
}

// 15.2.3.2 Object.getPrototypeOf ( O )
inline JSValue getPrototypeOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 1 || !vals[0].IsObject()) {
    e = error::TypeError();
    return JSValue();
  }
  return js_object::Prototype(vals[0]);
}

// 15.2.3.3 Object.getOwnPropertyDescriptor ( O, P )
inline JSValue getOwnPropertyDescriptor(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 1 || !vals[0].IsObject()) {
    e = error::TypeError(u"Object.create called on non-object");
    return JSValue();
  }
  JSValue name = vals.size() < 2 ?
    ::es::ToString(e, undefined::New()) : ::es::ToString(e, vals[1]);
  JSValue desc = GetOwnProperty(vals[0], name);
  return FromPropertyDescriptor(desc);
}

inline JSValue getOwnPropertyNames(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue defineProperty(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 1 || !vals[0].IsObject()) {
    e = error::TypeError(u"Object.defineProperty called on non-object");
    return JSValue();
  }
  if (vals.size() < 2) {
    e = error::TypeError(u"Object.defineProperty need 3 arguments");
    return JSValue();
  }
  JSValue name = ::es::ToString(e, vals[1]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue desc = ToPropertyDescriptor(e, vals[2]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  DefineOwnProperty(e, vals[0], name, desc, true);
  return vals[0];
}

inline JSValue defineProperties(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue seal(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue freeze(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue preventExtensions(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0 || !vals[0].IsObject()) {
    e = error::TypeError(u"Object.preventExtensions called on non-object");
    return JSValue();
  }
  js_object::SetExtensible(vals[0], false);
  return vals[0];
}

inline JSValue isSealed(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue isFrozen(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue isExtensible(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 1 || !vals[0].IsObject()) {
    e = error::TypeError(u"Object.isExtensible called on non-object");
    return JSValue();
  }
  return boolean::New(js_object::Extensible(vals[0]));
}

inline JSValue keys(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

// ES6
inline JSValue setPrototypeOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 2) {
    e = error::TypeError(u"Object.preventExtensions need 2 arguments");
    return JSValue(); 
  }
  CheckObjectCoercible(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (!(vals[1].IsNull() || vals[1].IsObject())) {
    e = error::TypeError(u"");
    return JSValue();
  }
  if (!vals[0].IsObject()) {
    return vals[0];
  }
  js_object::SetPrototype(vals[0], vals[1]);
  return vals[0];
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Object() { [native code] }");
}

inline JSValue create(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 1 || (!vals[0].IsObject() && !vals[0].IsNull())) {
    e = error::TypeError(u"Object.create called on non-object");
    return JSValue();
  }
  JSValue obj = object_object::New();
  js_object::SetPrototype(obj, vals[0]);
  if (vals.size() > 1 && !vals[1].IsUndefined()) {
    object_constructor::defineProperties(e, this_arg, vals);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  return obj;
}

}  // namespace object_constructor

JSValue Call__ObjectConstructor(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Construct__ObjectConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_OBJECT_OBJECT