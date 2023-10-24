#ifndef ES_ERROR_H
#define ES_ERROR_H

#include <string>

#include <es/types/base.h>

namespace es {
namespace error {

enum ErrorType {
  E_OK = 0,
  E_EVAL,
  E_RANGE,
  E_REFERENCE,
  E_SYNTAX,
  E_TYPE,
  E_URI,
  E_NATIVE,
};

constexpr size_t kErrorTypeOffset = 0;
constexpr size_t kValueOffset = kErrorTypeOffset + kSizeTSize;

inline JSValue New(ErrorType t, JSValue val, uint8_t flag) {
  JSValue jsval(JS_UNINIT);
  std::cout << "enter error" << std::endl;
  jsval.handle() = HeapObject::New(kSizeTSize + sizeof(JSValue), flag);

  SET_VALUE(jsval.handle().val(), kErrorTypeOffset, t, ErrorType);
  SET_JSVALUE(jsval.handle().val(), kValueOffset, val);

  jsval.SetType(ERROR);
  return jsval;
}

// TODO(zhuzilin) Fix memory leakage here.
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

ErrorType error_type(JSValue e) { return READ_VALUE(e.handle().val(), kErrorTypeOffset, ErrorType); }
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