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

  static Handle<JSValue> New(Type type, size_t size, uint8_t flag = 0) {
    Handle<HeapObject> mem = HeapObject::New(kIntSize + size, flag);
    SET_VALUE(mem.val(), kTypeOffset, type, Type);
    return Handle<JSValue>(mem);
  }

  inline Type type() { return READ_VALUE(this, kTypeOffset, Type); }
  inline bool IsJSValue() override { return true; }
  inline bool IsLanguageType() { return type() < LANG_TO_SPEC; }
  inline bool IsSpecificationType() { return type() > LANG_TO_SPEC; }
  inline bool IsPrimitive() { return type() < JS_OBJECT; }

  inline bool IsUndefined() { return type() == JS_UNDEFINED; }
  inline bool IsNull() { return type() == JS_NULL; }
  inline bool IsBool() { return type() == JS_BOOL; }
  inline bool IsString() { return type() == JS_STRING; }
  inline bool IsNumber() { return type() == JS_NUMBER; }
  inline bool IsObject() { return type() == JS_OBJECT; }

  inline bool IsNumberObject();
  inline bool IsArrayObject();
  inline bool IsRegExpObject();
  inline bool IsErrorObject();

  inline bool IsReference() { return type() == JS_REF; }
  inline bool IsPropertyDescriptor() { return type() == JS_PROP_DESC; }
  inline bool IsEnvironmentRecord() { return type() == JS_ENV_REC; }
  inline bool IsLexicalEnvironment() { return type() == JS_LEX_ENV; }

  inline bool IsPrototype() { return IsNull() || IsObject(); }

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

 protected:
  static constexpr size_t kTypeOffset = kHeapObjectOffset;
  static constexpr size_t kJSValueOffset = kTypeOffset + kIntSize;
};

class Undefined : public JSValue {
 public:
  static Handle<Undefined> Instance() {
    static Handle<Undefined> singleton = Undefined::New(GCFlag::CONST);
    return singleton;
  }

  inline std::string ToString() override { return "Undefined"; }
  inline std::vector<HeapObject**> Pointers() override { return {}; }

 private:
  static Handle<Undefined> New(uint8_t flag) {
    Handle<JSValue> jsval = JSValue::New(JS_UNDEFINED, 0, flag);
    new (jsval.val()) Undefined();
    return Handle<Undefined>(jsval);
  }
};

class Null : public JSValue {
 public:
  static Handle<Null> Instance() {
    static Handle<Null> singleton = Null::New(GCFlag::CONST);
    return singleton;
  }

  std::string ToString() override { return "Null"; }
  inline std::vector<HeapObject**> Pointers() override { return {}; }

 private:
  static Handle<Null> New(uint8_t flag) {
    Handle<JSValue> jsval = JSValue::New(JS_NULL, 0, flag);
    new (jsval.val()) Null();
    return Handle<Null>(jsval);
  }
};

class Bool : public JSValue {
 public:
  static Handle<Bool> True() {
    static Handle<Bool> singleton = Bool::New(true, GCFlag::CONST);
    return singleton;
  }
  static Handle<Bool> False() {
    static Handle<Bool> singleton = Bool::New(false, GCFlag::CONST);
    return singleton;
  }

  static Handle<Bool> Wrap(bool val) {
    return val ? True() : False();
  }

  inline bool data() { return READ_VALUE(this, kJSValueOffset, bool); }

  inline std::string ToString() override { return data() ? "true" : "false"; }
  inline std::vector<HeapObject**> Pointers() override { return {}; }

 private:
  static Handle<Bool> New(bool val, uint8_t flag) {
    Handle<JSValue> jsval = JSValue::New(JS_BOOL, kBoolSize, flag);
    SET_VALUE(jsval.val(), kJSValueOffset, val, bool);
    new (jsval.val()) Bool();
    return Handle<Bool>(jsval);
  }
};

class String : public JSValue {
 public:
  static Handle<String> New(std::u16string data, uint8_t flag = 0) {
    std::cout << "String::New " << log::ToString(data) << std::endl;
    Handle<JSValue> jsval = JSValue::New(JS_STRING, kSizeTSize + data.size() * kChar16Size, flag);
    SET_VALUE(jsval.val(), kLengthOffset, data.size(), size_t);
    memcpy(PTR(jsval.val(), kStringDataOffset), data.data(), data.size() * kChar16Size);
    new (jsval.val()) String();
    return Handle<String>(jsval);
  }

  static Handle<String> New(char16_t* data, uint8_t flag = 0) {
    std::cout << "String::New" << std::endl;
    size_t size = std::char_traits<char16_t>::length(data);
    Handle<JSValue> jsval = JSValue::New(JS_STRING, kSizeTSize + size * kChar16Size, flag);
    SET_VALUE(jsval.val(), kLengthOffset, size, size_t);
    memcpy(PTR(jsval.val(), kStringDataOffset), data, size * kChar16Size);
    new (jsval.val()) String();
    return Handle<String>(jsval);
  }

  static Handle<String> New(size_t n) {
    Handle<JSValue> jsval = JSValue::New(JS_STRING, kSizeTSize + n * kChar16Size);
    SET_VALUE(jsval.val(), kLengthOffset, n, int);
    new (jsval.val()) String();
    return Handle<String>(jsval);
  }

  std::u16string data() { return std::u16string(c_str(), size()); }
  char16_t* c_str() { return TYPED_PTR(this, kStringDataOffset, char16_t); }
  size_t size() { return READ_VALUE(this, kLengthOffset, size_t); }

  char16_t& operator [](int index) {
    return c_str()[index];
  }

  Handle<String> substr(size_t pos = 0, size_t len = std::u16string::npos) {
    if (len == std::u16string::npos) {
      len = size() - pos;
    }
    Handle<String> substring = String::New(len);
    memcpy(
      PTR(substring.val(), kStringDataOffset),
      PTR(this, kStringDataOffset + pos * kChar16Size),
      len * kChar16Size
    );
    return substring;
  }

  static Handle<String> Empty() {
    static Handle<String> singleton = String::New(u"", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Undefined() {
    static Handle<String> singleton = String::New(u"undefined", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Null() {
    static Handle<String> singleton = String::New(u"null", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> True() {
    static Handle<String> singleton = String::New(u"true", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> False() {
    static Handle<String> singleton = String::New(u"false", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> NaN() {
    static Handle<String> singleton = String::New(u"NaN", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Zero() {
    static Handle<String> singleton = String::New(u"0", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Infinity() {
    static Handle<String> singleton = String::New(u"Infinity", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Prototype() {
    static Handle<String> singleton = String::New(u"prototype", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Length() {
    static Handle<String> singleton = String::New(u"length", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Value() {
    static Handle<String> singleton = String::New(u"value", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Writable() {
    static Handle<String> singleton = String::New(u"writable", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Get() {
    static Handle<String> singleton = String::New(u"get", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Set() {
    static Handle<String> singleton = String::New(u"set", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Arguments() {
    static Handle<String> singleton = String::New(u"arguments", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Enumerable() {
    static Handle<String> singleton = String::New(u"enumerable", GCFlag::CONST);
    return singleton;
  }

  static Handle<String> Configurable() {
    static Handle<String> singleton = String::New(u"configurable", GCFlag::CONST);
    return singleton;
  }

  inline std::string ToString() override { return log::ToString(data()); }
  inline std::vector<HeapObject**> Pointers() override { return {}; }

 private:
  static constexpr size_t kLengthOffset = kJSValueOffset;
  static constexpr size_t kStringDataOffset = kLengthOffset + kSizeTSize;
};

bool operator ==(String& a, String& b) {
  if (a.size() != b.size()) return false;
  size_t size = a.size();
  for (size_t i = 0; i < size; i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

bool operator !=(String& a, String& b) {
  return !(a == b);
}

bool operator <(String& a, String& b) {
  if (a.size() != b.size()) return false;
  size_t size = a.size();
  if (b.size() < size)
    size = b.size();
  for (size_t i = 0; i < size; i++) {
    if (a[i] != b[i])
      return a[i] < b[i];
  }
  return a.size() < b.size();
}

class Number : public JSValue {
 public:
  static Handle<Number> New(double data, uint8_t flag = 0) {
    std::cout << "Number::New" << std::endl;
    Handle<JSValue> jsval = JSValue::New(JS_NUMBER, kDoubleSize, flag);
    SET_VALUE(jsval.val(), kJSValueOffset, data, double);
    new (jsval.val()) Number();
    return Handle<Number>(jsval);
  }

  static Handle<Number> NaN() {
    static Handle<Number> singleton = Number::New(nan(""), GCFlag::CONST);
    return singleton;
  }

  static Handle<Number> PositiveInfinity() {
    static Handle<Number> singleton = Number::New(std::numeric_limits<double>::infinity(), GCFlag::CONST);
    return singleton;
  }

  static Handle<Number> NegativeInfinity() {
    static Handle<Number> singleton = Number::New(-std::numeric_limits<double>::infinity(), GCFlag::CONST);
    return singleton;
  }

  static Handle<Number> Zero() {
    static Handle<Number> singleton = Number::New(0.0, GCFlag::CONST);
    return singleton;
  }

  static Handle<Number> NegativeZero() {
    static Handle<Number> singleton = Number::New(-0.0, GCFlag::CONST);
    return singleton;
  }

  static Handle<Number> One() {
    static Handle<Number> singleton = Number::New(1.0, GCFlag::CONST);
    return singleton;
  }

  inline bool IsInfinity() { return isinf(data()); }
  inline bool IsPositiveInfinity() { return data() == std::numeric_limits<double>::infinity(); }
  inline bool IsNegativeInfinity() { return data() == -std::numeric_limits<double>::infinity(); }
  inline bool IsNaN() { return isnan(data()); }

  inline double data() { return READ_VALUE(this, kJSValueOffset, double); }

  inline std::string ToString() override { return std::to_string(data()); }
  inline std::vector<HeapObject**> Pointers() override { return {}; }
};

}  // namespace es

#endif  // ES_TYPES_BASE_H