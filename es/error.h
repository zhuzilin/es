#ifndef ES_ERROR_H
#define ES_ERROR_H

namespace es {

class Error {
 public:
  enum Type {
    E_EVAL = 0,
    E_RANGE,
    E_REFERENCE,
    E_SYNTAX,
    E_TYPE,
    E_URI,
    E_NATIVE,
  };

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

  static Error* NativeError() {
    static Error e(E_NATIVE);
    return &e;
  }

  Type type() { return type_; }

 private:
  Error(Type t) : type_(t) {}
  Type type_;
};

}  // namespace es

#endif  // ES_ERROR_H