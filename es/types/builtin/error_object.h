#ifndef ES_TYPES_BUILTIN_ERROR_OBJECT
#define ES_TYPES_BUILTIN_ERROR_OBJECT

#include <es/types/object.h>

namespace es {

namespace error_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Error", true, JSValue(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = error_proto::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (!val.IsObject()) {
    e = error::TypeError(u"Error.prototype.toString called with non-object value");
    return JSValue();
  }
  // TODO(zhuzilin) add name
  std::u16string name = u"Error";
  JSValue msg = Get(e, val, string::New(u"message"));
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (msg.IsUndefined())
    return undefined::New();
  std::u16string msg_str = string::data(::es::ToString(e, msg));
  if (unlikely(!error::IsOk(e))) return JSValue();
  return string::New(name + u": " + msg_str);
}


}  // namespace error_proto

namespace error_object {

constexpr size_t kErrorOffset = js_object::kJSObjectOffset;

inline JSValue New(JSValue e) {
  ASSERT(e.IsError());
  JSValue jsobj = js_object::New(
    u"Error", true, JSValue(), false, false, nullptr, sizeof(JSValue)
  );

  SET_JSVALUE(jsobj.handle().val(), kErrorOffset, e);

  jsobj.SetType(OBJ_ERROR);
  js_object::SetPrototype(jsobj, error_proto::Instance());
  AddValueProperty(jsobj, u"message", error::value(e), true, false, false);
  return jsobj;
}

inline JSValue e(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kErrorOffset); }
inline error::ErrorType ErrorType(JSValue jsval) { return error::error_type(e(jsval)); }
inline JSValue ErrorValue(JSValue jsval) { return error::value(e(jsval)); }

}  // namespace error_object

namespace error_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Error", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_ERROR_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = error_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Error() { [native code] }");
}

}  // namespace error_constructor

JSValue Construct__ErrorConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ERROR_OBJECT