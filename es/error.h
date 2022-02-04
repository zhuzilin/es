#ifndef ES_ERROR_H
#define ES_ERROR_H

#include <string>

#include <es/types/base.h>

namespace es {

class Error : public HeapObject {
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
    static Handle<Error> singleton = Error::New(E_OK, String::Empty(), GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Error>& Empty() {
    static Handle<Error> singleton = Error::New(E_OK, Handle<JSValue>(), GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Error> EvalError() {
    static Handle<Error> singleton = Error::New(E_EVAL, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetMessage(u"");
    return singleton;
  }

  static Handle<Error> RangeError(std::u16string message) {
    static Handle<Error> singleton = Error::New(E_EVAL, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> ReferenceError(std::u16string message) {
    static Handle<Error> singleton = Error::New(E_REFERENCE, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> SyntaxError(std::u16string message) {
    static Handle<Error> singleton = Error::New(E_SYNTAX, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> TypeError(std::u16string message = u"") {
    static Handle<Error> singleton = Error::New(E_TYPE, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetMessage(message);
    return singleton;
  }

  static Handle<Error> UriError() {
    static Handle<Error> singleton = Error::New(E_URI, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetMessage(u"");
    return singleton;
  }

  static Handle<Error> NativeError(Handle<JSValue> val) {
    static Handle<Error> singleton = Error::New(E_NATIVE, Handle<JSValue>(), GCFlag::SINGLE);
    singleton.val()->SetValue(val);
    return singleton;
  }

  ErrorType type() { return READ_VALUE(this, kErrorTypeOffset, ErrorType); }
  Handle<JSValue> value() { return READ_HANDLE_VALUE(this, kValueOffset, JSValue); }
  void SetValue(Handle<JSValue> val) { SET_HANDLE_VALUE(this, kValueOffset, val, JSValue); }

  void SetMessage(std::u16string message) {
    if (message == u"") {
      SetValue(String::Empty());
    } else {
      SetValue(String::New(message));
    }
  }

  bool IsOk() { return type() == E_OK; }

  std::string ToString() override {
    return IsOk() ?
      "ok" :
      ("error(" + (READ_VALUE(this, kValueOffset, String*))->ToString() + ")");
  }

  virtual std::vector<HeapObject**> Pointers() override {
    return {HEAP_PTR(kValueOffset)};
  }

 private:
  static Handle<Error> New(ErrorType t, Handle<JSValue> val, uint8_t flag) {
#ifdef GC_DEBUG
    if (log::Debugger::On())
      std::cout << "Error::New " << std::endl;
#endif
    Handle<HeapObject> heap_obj = HeapObject::New(kIntSize + kPtrSize, flag);

    SET_VALUE(heap_obj.val(), kErrorTypeOffset, t, ErrorType);
    SET_HANDLE_VALUE(heap_obj.val(), kValueOffset, val, JSValue);

    new (heap_obj.val()) Error();
    return Handle<Error>(heap_obj);
  }

  static constexpr size_t kErrorTypeOffset = kHeapObjectOffset;
  static constexpr size_t kValueOffset = kErrorTypeOffset + kIntSize;
};

}  // namespace es

#endif  // ES_ERROR_H