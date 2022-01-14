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

  // TODO(zhuzilin) Fix memory leakage here.
  static Error* Ok() {
    return new Error(E_OK);
  }

  static Error* EvalError() {
    return new Error(E_EVAL);
  }

  static Error* RangeError() {
    return new Error(E_RANGE);
  }

  static Error* ReferenceError() {
    return new Error(E_REFERENCE);
  }

  static Error* SyntaxError() {
    return new Error(E_SYNTAX);
  }

  static Error* TypeError(std::u16string message = u"") {
    return new Error(E_TYPE, message);
  }

  static Error* UriError() {
    return new Error(E_URI);
  }

  static Error* NativeError(std::u16string message) {
    return new Error(E_NATIVE, message);
  }

  Type type() { return type_; }

  bool IsOk() { return type_ == E_OK; }

  std::u16string message() {
    return message_;
  }

  std::string ToString() { return Ok() ? "ok" : "error"; }

 private:
  Error(Type t, std::u16string message = u"") :
    type_(t), message_(message) {}

  Type type_;
  std::u16string message_;
};

}  // namespace es

#endif  // ES_ERROR_H