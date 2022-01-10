#ifndef ES_TYPES_BASE_H
#define ES_TYPES_BASE_H

#include <string>
#include <string_view>
#include <unordered_map>

#include <es/error.h>

namespace es {

class JSValue {
 public:
  enum Type {
    JS_UNDEFINED = 0,
    JS_NULL,
    JS_BOOL,
    JS_STRING,
    JS_NUMBER,
    JS_OBJECT,

    LANG_TO_SPEC,

    JS_REF,
    JS_LIST,
    JS_PROP_DESC,
    JS_PROP_IDEN,
    JS_LEX_ENV,
    JS_ENV_REC,
  };

  JSValue(Type type) : type_(type) {}

  inline Type type() { return type_; }
  inline bool IsLanguageType() { return type_ < LANG_TO_SPEC; }
  inline bool IsSpecificationType() { return type_ > LANG_TO_SPEC; }
  inline bool IsPrimitive() { return type_ < JS_OBJECT; }

  inline bool IsUndefined() { return type_ == JS_UNDEFINED; }
  inline bool IsNull() { return type_ == JS_NULL; }
  inline bool IsBool() { return type_ == JS_BOOL; }
  inline bool IsString() { return type_ == JS_STRING; }
  inline bool IsNumber() { return type_ == JS_NUMBER; }
  inline bool IsObject() { return type_ == JS_OBJECT; }

  inline bool IsReference() { return type_ == JS_REF; }
  inline bool IsPropertyDescriptor() { return type_ == JS_PROP_DESC; }
  inline bool IsPropertyIdentifier() { return type_ == JS_PROP_IDEN; }
  inline bool IsLexicalEnvironment() { return type_ == JS_LEX_ENV; }
  inline bool IsEnvironmentRecord() { return type_ == JS_ENV_REC; }

  void CheckObjectCoercible(Error* e) {
    if (IsUndefined() || IsNull()) {
      e = Error::TypeError();
    }
  }
  virtual bool IsCallable() {
    // JSObject need to implement its own IsCallable
    assert(!IsObject());
    return false;
  }

 private:
  Type type_;
};

class Undefined : public JSValue {
 public:
  static Undefined* Instance() {
    static Undefined singleton;
    return &singleton;
  }

 private:
  Undefined() : JSValue(JS_UNDEFINED) {}
};

class Null : public JSValue {
 public:
  static Null* Instance() {
    static Null singleton;
    return &singleton;
  }

 private:
  Null() : JSValue(JS_NULL) {}
};

class Bool : public JSValue {
 public:
  static Bool* True() {
    static Bool singleton(true);
    return &singleton;
  }
  static Bool* False() {
    static Bool singleton(false);
    return &singleton;
  }

  static Bool* Wrap(bool val) {
    return val ? True() : False();
  }

  bool data() { return data_; }

 private:
  Bool(bool data) : JSValue(JS_BOOL), data_(data) {}

  bool data_;
};

class String : public JSValue {
 public:
  String(std::u16string data) : JSValue(JS_STRING), data_(data) {}
  std::u16string data() { return data_; }

 private:
  std::u16string data_;
};

class Number : public JSValue {
 public:
  Number(double data, int8_t infinity_flag = 0) :
    JSValue(JS_NUMBER), infinity_flag_(infinity_flag), data_(data) {}

  static constexpr double NaN = 9007199254740990.0;  // (1.0 << 53) - 2

  static Number* Zero() {
    static Number singleton(0);
    return &singleton;
  }

  bool IsInfinity() { return infinity_flag_ != 0; }
  bool IsPositiveInfinity() { return infinity_flag_ > 0; }
  bool IsNegativeInfinity() { return infinity_flag_ < 0; }
  bool IsNaN() { return data_ == NaN; }

  double data() { return data_; }

 private:
  double data_;
  char infinity_flag_;
};

}  // namespace es

#endif  // ES_TYPES_BASE_H