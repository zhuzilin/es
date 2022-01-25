#ifndef ES_TYPES_BASE_H
#define ES_TYPES_BASE_H

#include <math.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include <es/gc/heap_object.h>
#include <es/error.h>

namespace es {

class JSValue : public HeapObject {
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
    JS_PROP_DESC,
    JS_ENV_REC,
    JS_LEX_ENV,

    NUM_TYPES,
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

  inline bool IsNumberObject();
  inline bool IsArrayObject();
  inline bool IsRegExpObject();
  inline bool IsErrorObject();

  inline bool IsReference() { return type_ == JS_REF; }
  inline bool IsPropertyDescriptor() { return type_ == JS_PROP_DESC; }
  inline bool IsEnvironmentRecord() { return type_ == JS_ENV_REC; }
  inline bool IsLexicalEnvironment() { return type_ == JS_LEX_ENV; }

  virtual std::string ToString() = 0;

  void CheckObjectCoercible(Error* e) {
    if (IsUndefined() || IsNull()) {
      *e = *Error::TypeError(u"undefined or null is not coercible");
    }
  }
  virtual bool IsCallable() {
    // JSObject need to implement its own IsCallable
    assert(!IsObject());
    return false;
  }

  virtual bool IsConstructor() {
    // JSObject need to implement its own IsConstructor
    assert(!IsObject());
    return false;
  }


 private:
  Type type_;
};

class Undefined : public JSValue {
 public:
  static Undefined* Instance() {
    static Undefined* singleton = new Undefined();
    return singleton;
  }

  inline std::string ToString() override { return "Undefined"; }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  Undefined() : JSValue(JS_UNDEFINED) {}
};

class Null : public JSValue {
 public:
  static Null* Instance() {
    static Null* singleton = new Null();
    return singleton;
  }
  std::string ToString() override { return "Null"; }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  Null() : JSValue(JS_NULL) {}
};

class Bool : public JSValue {
 public:
  static Bool* True() {
    static Bool* singleton = new Bool(true);
    return singleton;
  }
  static Bool* False() {
    static Bool* singleton = new Bool(false);
    return singleton;
  }

  static Bool* Wrap(bool val) {
    return val ? True() : False();
  }

  inline bool data() { return data_; }

  inline std::string ToString() override { return data_ ? "true" : "false"; }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  Bool(bool data) : JSValue(JS_BOOL), data_(data) {}

  bool data_;
};

class String : public JSValue {
 public:
  String(std::u16string data) : JSValue(JS_STRING), data_(data) {}
  std::u16string data() { return data_; }

  static String* Empty() {
    static String* singleton = new String(u"");
    return singleton;
  }

  static String* Undefined() {
    static String* singleton = new String(u"undefined");
    return singleton;
  }

  static String* Null() {
    static String* singleton = new String(u"null");
    return singleton;
  }

  static String* True() {
    static String* singleton = new String(u"true");
    return singleton;
  }

  static String* False() {
    static String* singleton = new String(u"false");
    return singleton;
  }

  static String* NaN() {
    static String* singleton = new String(u"NaN");
    return singleton;
  }

  static String* Zero() {
    static String* singleton = new String(u"0");
    return singleton;
  }

  static String* Infinity() {
    static String* singleton = new String(u"Infinity");
    return singleton;
  }

  inline std::string ToString() override { return log::ToString(data_); }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  std::u16string data_;
};

class Number : public JSValue {
 public:
  Number(double data) :
    JSValue(JS_NUMBER), data_(data) {}

  static Number* NaN() {
    static Number* singleton = new Number(nan(""));
    return singleton;
  }

  static Number* PositiveInfinity() {
    static Number* singleton = new Number(std::numeric_limits<double>::infinity());
    return singleton;
  }

  static Number* NegativeInfinity() {
    static Number* singleton = new Number(-std::numeric_limits<double>::infinity());
    return singleton;
  }

  static Number* Zero() {
    static Number* singleton = new Number(0.0);
    return singleton;
  }

  static Number* NegativeZero() {
    static Number* singleton = new Number(-0.0);
    return singleton;
  }

  static Number* One() {
    static Number* singleton = new Number(1.0);
    return singleton;
  }

  inline bool IsInfinity() { return isinf(data_); }
  inline bool IsPositiveInfinity() { return data_ == std::numeric_limits<double>::infinity(); }
  inline bool IsNegativeInfinity() { return data_ == -std::numeric_limits<double>::infinity(); }
  inline bool IsNaN() { return isnan(data_); }

  inline double data() { return data_; }

  inline std::string ToString() override { return std::to_string(data_); }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  double data_;
};

}  // namespace es

#endif  // ES_TYPES_BASE_H