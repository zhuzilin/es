#ifndef ES_ERROR_H
#define ES_ERROR_H

#include <string>

#include <es/types/base.h>

namespace es {

class Error : public JSValue {
 public:
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

  // TODO(zhuzilin) Fix memory leakage here.
  static Handle<Error> Ok() {
    static Handle<Error> singleton = Error::New<E_OK, GCFlag::SINGLE>(String::Empty());
    return singleton;
  }

  static Handle<Error>& Empty() {
    static Handle<Error> singleton = Error::New<E_OK, GCFlag::SINGLE>(Handle<JSValue>());
    return singleton;
  }

  static Handle<Error> EvalError() {
    static Handle<Error> singleton = Error::New<E_EVAL, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetMessage(u"");
    return singleton;
  }

  static Handle<Error> RangeError(std::u16string message) {
    static Handle<Error> singleton = Error::New<E_EVAL, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> ReferenceError(std::u16string message) {
    static Handle<Error> singleton = Error::New<E_REFERENCE, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> SyntaxError(std::u16string message) {
    static Handle<Error> singleton = Error::New<E_SYNTAX, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetMessage(message);
    return singleton;
  }

  // Only used in parser
  static Handle<Error> SyntaxErrorConst(std::u16string message) {
    std::cout << "SyntaxError const" << std::endl;
    Handle<Error> error = Error::New<E_SYNTAX, GCFlag::CONST>(Handle<JSValue>());
    Handle<String> msg = message == u"" ? String::Empty() : String::New<GCFlag::CONST>(message);
    error.val()->SetValue(msg);
    return error;
  }

  static Handle<Error> TypeError(std::u16string message = u"") {
    static Handle<Error> singleton = Error::New<E_TYPE, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> UriError() {
    static Handle<Error> singleton = Error::New<E_URI, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetMessage(u"");
    return singleton;
  }

  static Handle<Error> NativeError(Handle<JSValue> val) {
    static Handle<Error> singleton = Error::New<E_NATIVE, GCFlag::SINGLE>(Handle<JSValue>());
    singleton.val()->SetValue(val);
    return singleton;
  }

  ErrorType error_type() { return READ_VALUE(this, kErrorTypeOffset, ErrorType); }
  Handle<JSValue> value() { return READ_HANDLE_VALUE(this, kValueOffset, JSValue); }
  void SetValue(Handle<JSValue> val) { SET_HANDLE_VALUE(this, kValueOffset, val, JSValue); }

  void SetMessage(std::u16string message) {
    if (message == u"") {
      SetValue(String::Empty());
    } else {
      SetValue(String::New(message));
    }
  }

  bool IsOk() { return error_type() == E_OK; }
  bool IsNativeError() { return error_type() == E_NATIVE; }

 private:
  template<ErrorType t, flag_t flag = 0>
  static Handle<Error> New(Handle<JSValue> val) {
#ifdef GC_DEBUG
    TEST_LOG("Error::New\n");
#endif
    Handle<JSValue> jsval = HeapObject::New<kUint32Size + kPtrSize, flag>();

    SET_VALUE(jsval.val(), kErrorTypeOffset, t, ErrorType);
    SET_HANDLE_VALUE(jsval.val(), kValueOffset, val, JSValue);
    jsval.val()->SetType(ERROR);

    return Handle<Error>(jsval);
  }

 public:
  static constexpr size_t kErrorTypeOffset = HeapObject::kHeapObjectOffset;
  static constexpr size_t kValueOffset = kErrorTypeOffset + kUint32Size;
};

}  // namespace es

#endif  // ES_ERROR_H