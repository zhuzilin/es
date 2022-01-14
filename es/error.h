#ifndef ES_ERROR_H
#define ES_ERROR_H

#include <string>

namespace es {

class JSValue;

class Error {
 public:
  enum Type {
    E_OK = 0,
    E_EVAL,
    E_RANGE,
    E_REFERENCE,
    E_SYNTAX,
    E_TYPE,
    E_URI,
    E_NATIVE,
  };

  static Error* Ok() {
    static Error e(E_OK);
    return &e;
  }

  static Error* EvalError() {
    static Error e(E_EVAL);
    return &e;
  }

  static Error* RangeError() {
    static Error e(E_RANGE);
    return &e;
  }

  static Error* ReferenceError() {
    static Error e(E_REFERENCE);
    return &e;
  }

  static Error* SyntaxError() {
    static Error e(E_SYNTAX);
    return &e;
  }

  static Error* TypeError() {
    static Error e(E_TYPE);
    return &e;
  }

  static Error* UriError() {
    static Error e(E_URI);
    return &e;
  }

  static Error* NativeError(JSValue* val) {
    return new Error(E_NATIVE, val);
  }

  Type type() { return type_; }

  bool IsOk() { return type_ == E_OK; }
  bool IsNative() {
    assert(val_ != nullptr);
    return type_ == E_NATIVE;
  }

  JSValue* val() {
    assert(val_ != nullptr);
    return val_;
  }

  std::string ToString() { return Ok() ? "ok" : "error"; }

 private:
  Error(Type t, JSValue* val = nullptr) : type_(t), val_(val) {}
  Type type_;
  JSValue* val_;
};

}  // namespace es

#endif  // ES_ERROR_H