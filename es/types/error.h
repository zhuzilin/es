#ifndef ES_ERROR_H
#define ES_ERROR_H

#include <string>

#include <es/types/base.h>

namespace es {
namespace error {

constexpr size_t kValueOffset = 0;

inline JSValue New(ErrorType t, JSValue val, uint8_t flag) {
  JSValue jsval(JS_UNINIT);
  jsval.handle() = HeapObject::New(sizeof(JSValue), flag);

  jsval.header_.placeholder_.error_type_ = t;
  SET_JSVALUE(jsval.handle().val(), kValueOffset, val);

  jsval.SetType(ERROR);
  return jsval;
}

inline JSValue Ok() {
  static JSValue singleton = New(E_OK, string::Empty(), GCFlag::SINGLE);
  return singleton;
}

inline JSValue Empty() {
  static JSValue singleton = New(E_OK, null::New(), GCFlag::SINGLE);
  return singleton;
}

inline JSValue EvalError() {
  static JSValue singleton = New(E_EVAL, string::Empty(), GCFlag::SINGLE);
  return singleton;
}

inline JSValue RangeError(std::u16string message) {
  static JSValue singleton = New(E_EVAL, string::New(message), GCFlag::SINGLE);
  return singleton;
}

inline JSValue ReferenceError(std::u16string message) {
  static JSValue singleton = New(E_REFERENCE, string::New(message), GCFlag::SINGLE);
  return singleton;
}

inline JSValue SyntaxError(std::u16string message) {
  static JSValue singleton = New(E_SYNTAX, string::New(message), GCFlag::SINGLE);
  return singleton;
}

inline JSValue TypeError(std::u16string message = u"") {
  static JSValue singleton = New(E_TYPE, string::New(message), GCFlag::SINGLE);
  return singleton;
}

inline JSValue UriError() {
  static JSValue singleton = New(E_URI, string::Empty(), GCFlag::SINGLE);
  return singleton;
}

inline JSValue NativeError(JSValue val) {
  static JSValue singleton = New(E_NATIVE, val, GCFlag::SINGLE);
  return singleton;
}

ErrorType error_type(JSValue e) { return e.header_.placeholder_.error_type_; }
JSValue value(JSValue e) { return GET_JSVALUE(e.handle().val(), kValueOffset); }
void SetValue(JSValue& e, JSValue val) { SET_JSVALUE(e.handle().val(), kValueOffset, val); }

void SetMessage(JSValue& e, std::u16string message) {
  if (message == u"") {
    SetValue(e, string::Empty());
  } else {
    SetValue(e, string::New(message));
  }
}

bool IsOk(JSValue e) { return error_type(e) == E_OK; }
bool IsNativeError(JSValue e) { return error_type(e) == E_NATIVE; }

}  // namespace error
}  // namespace es

#endif  // ES_ERROR_H