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

  static JSValue* New(Type type, size_t size, uint8_t flag = 0) {
    void* mem = HeapObject::New(kIntSize + size, flag);
    SET_VALUE(mem, kTypeOffset, type, Type);
    return static_cast<JSValue*>(mem);
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
  static Undefined* Instance() {
    static Undefined* singleton = Undefined::New(GCFlag::CONST);
    return singleton;
  }

  inline std::string ToString() override { return "Undefined"; }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  static Undefined* New(uint8_t flag) {
    JSValue* jsval = JSValue::New(JS_UNDEFINED, 0, flag);
    return new (jsval) Undefined();
  }
};

class Null : public JSValue {
 public:
  static Null* Instance() {
    static Null* singleton = Null::New(GCFlag::CONST);
    return singleton;
  }

  std::string ToString() override { return "Null"; }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  static Null* New(uint8_t flag) {
    JSValue* jsval = JSValue::New(JS_NULL, 0, flag);
    return new (jsval) Null();
  }
};

class Bool : public JSValue {
 public:
  static Bool* True() {
    static Bool* singleton = Bool::New(true, GCFlag::CONST);
    return singleton;
  }
  static Bool* False() {
    static Bool* singleton = Bool::New(false, GCFlag::CONST);
    return singleton;
  }

  static Bool* Wrap(bool val) {
    return val ? True() : False();
  }

  inline bool data() { return READ_VALUE(this, kJSValueOffset, bool); }

  inline std::string ToString() override { return data() ? "true" : "false"; }
  inline std::vector<void*> Pointers() override { return {}; }

 private:
  static Bool* New(bool val, uint8_t flag) {
    JSValue* jsval = JSValue::New(JS_BOOL, kBoolSize, flag);
    SET_VALUE(jsval, kJSValueOffset, val, bool);
    return new (jsval) Bool();
  }
};

class String : public JSValue {
 public:
  static String* New(std::u16string data, uint8_t flag = 0) {
    std::cout << "String::New" << std::endl;
    JSValue* jsval = JSValue::New(JS_STRING, kSizeTSize + data.size() * kChar16Size, flag);
    SET_VALUE(jsval, kLengthOffset, data.size(), size_t);
    memcpy(PTR(jsval, kStringDataOffset), data.data(), data.size() * kChar16Size);
    return new (jsval) String();
  }

  static String* New(char16_t* data, uint8_t flag = 0) {
    std::cout << "String::New" << std::endl;
    size_t size = std::char_traits<char16_t>::length(data);
    JSValue* jsval = JSValue::New(JS_STRING, kSizeTSize + size * kChar16Size, flag);
    SET_VALUE(jsval, kLengthOffset, size, size_t);
    memcpy(PTR(jsval, kStringDataOffset), data, size * kChar16Size);
    return new (jsval) String();
  }

  static String* New(size_t n) {
    JSValue* jsval = JSValue::New(JS_STRING, kSizeTSize + n * kChar16Size);
    SET_VALUE(jsval, kLengthOffset, n, int);
    return new (jsval) String();
  }

  std::u16string data() { return std::u16string(c_str(), size()); }
  char16_t* c_str() { return TYPED_PTR(this, kStringDataOffset, char16_t); }
  size_t size() { return READ_VALUE(this, kLengthOffset, size_t); }

  char16_t& operator [](int index) {
    return c_str()[index];
  }

  String* substr(size_t pos = 0, size_t len = std::u16string::npos) {
    if (len == std::u16string::npos) {
      len = size() - pos;
    }
    String* substring = String::New(len);
    memcpy(
      PTR(substring, kStringDataOffset),
      PTR(this, kStringDataOffset + pos * kChar16Size),
      len * kChar16Size
    );
    return substring;
  }

  static String* Empty() {
    static String* singleton = String::New(u"", GCFlag::CONST);
    return singleton;
  }

  static String* Undefined() {
    static String* singleton = String::New(u"undefined", GCFlag::CONST);
    return singleton;
  }

  static String* Null() {
    static String* singleton = String::New(u"null", GCFlag::CONST);
    return singleton;
  }

  static String* True() {
    static String* singleton = String::New(u"true", GCFlag::CONST);
    return singleton;
  }

  static String* False() {
    static String* singleton = String::New(u"false", GCFlag::CONST);
    return singleton;
  }

  static String* NaN() {
    static String* singleton = String::New(u"NaN", GCFlag::CONST);
    return singleton;
  }

  static String* Zero() {
    static String* singleton = String::New(u"0", GCFlag::CONST);
    return singleton;
  }

  static String* Infinity() {
    static String* singleton = String::New(u"Infinity", GCFlag::CONST);
    return singleton;
  }

  static String* Prototype() {
    static String* singleton = String::New(u"prototype", GCFlag::CONST);
    return singleton;
  }

  static String* Length() {
    static String* singleton = String::New(u"length", GCFlag::CONST);
    return singleton;
  }

  static String* Value() {
    static String* singleton = String::New(u"value", GCFlag::CONST);
    return singleton;
  }

  static String* Writable() {
    static String* singleton = String::New(u"writable", GCFlag::CONST);
    return singleton;
  }

  static String* Get() {
    static String* singleton = String::New(u"get", GCFlag::CONST);
    return singleton;
  }

  static String* Set() {
    static String* singleton = String::New(u"set", GCFlag::CONST);
    return singleton;
  }

  static String* Arguments() {
    static String* singleton = String::New(u"arguments", GCFlag::CONST);
    return singleton;
  }

  static String* Enumerable() {
    static String* singleton = String::New(u"enumerable", GCFlag::CONST);
    return singleton;
  }

  static String* Configurable() {
    static String* singleton = String::New(u"configurable", GCFlag::CONST);
    return singleton;
  }

  inline std::string ToString() override { return log::ToString(data()); }
  inline std::vector<void*> Pointers() override { return {}; }

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
  static Number* New(double data, uint8_t flag = 0) {
    std::cout << "Number::New" << std::endl;
    JSValue* jsval = JSValue::New(JS_NUMBER, kDoubleSize, flag);
    SET_VALUE(jsval, kJSValueOffset, data, double);
    return new (jsval) Number();
  }

  static Number* NaN() {
    static Number* singleton = Number::New(nan(""), GCFlag::CONST);
    return singleton;
  }

  static Number* PositiveInfinity() {
    static Number* singleton = Number::New(std::numeric_limits<double>::infinity(), GCFlag::CONST);
    return singleton;
  }

  static Number* NegativeInfinity() {
    static Number* singleton = Number::New(-std::numeric_limits<double>::infinity(), GCFlag::CONST);
    return singleton;
  }

  static Number* Zero() {
    static Number* singleton = Number::New(0.0, GCFlag::CONST);
    return singleton;
  }

  static Number* NegativeZero() {
    static Number* singleton = Number::New(-0.0, GCFlag::CONST);
    return singleton;
  }

  static Number* One() {
    static Number* singleton = Number::New(1.0, GCFlag::CONST);
    return singleton;
  }

  inline bool IsInfinity() { return isinf(data()); }
  inline bool IsPositiveInfinity() { return data() == std::numeric_limits<double>::infinity(); }
  inline bool IsNegativeInfinity() { return data() == -std::numeric_limits<double>::infinity(); }
  inline bool IsNaN() { return isnan(data()); }

  inline double data() { return READ_VALUE(this, kJSValueOffset, double); }

  inline std::string ToString() override { return std::to_string(data()); }
  inline std::vector<void*> Pointers() override { return {}; }
};

}  // namespace es

#endif  // ES_TYPES_BASE_H