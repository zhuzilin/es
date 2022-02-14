#ifndef ES_TYPES_BASE_H
#define ES_TYPES_BASE_H

#include <math.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include <es/gc/heap_object.h>

namespace es {

class JSValue : public HeapObject {
 public:
  static Handle<JSValue> New(size_t size, flag_t flag = 0) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "JSValue::New " << " " << size << " " << "\n";
#endif
    Handle<HeapObject> mem = HeapObject::New(size, flag);
    return Handle<JSValue>(mem);
  }

  bool IsCallable();
  bool IsConstructor();

 public:
  static constexpr size_t kJSValueOffset = kHeapObjectOffset;
};

class Undefined : public JSValue {
 public:
  static Handle<Undefined> Instance() {
    static Handle<Undefined> singleton = Undefined::New(GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

 private:
  static Handle<Undefined> New(flag_t flag) {
    Handle<JSValue> jsval = JSValue::New(0, flag);

    jsval.val()->SetType(JS_UNDEFINED);
    return Handle<Undefined>(jsval);
  }
};

class Null : public JSValue {
 public:
  static Handle<Null> Instance() {
    static Handle<Null> singleton = Null::New(GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

 private:
  static Handle<Null> New(flag_t flag) {
    Handle<JSValue> jsval = JSValue::New(0, flag);

    jsval.val()->SetType(JS_NULL);
    return Handle<Null>(jsval);
  }
};

class Bool : public JSValue {
 public:
  static Handle<Bool> True() {
    static Handle<Bool> singleton = Bool::New(true, GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }
  static Handle<Bool> False() {
    static Handle<Bool> singleton = Bool::New(false, GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Bool> Wrap(bool val) {
    return val ? True() : False();
  }

  inline bool data() { return READ_VALUE(this, kJSValueOffset, bool); }

 private:
  static Handle<Bool> New(bool val, flag_t flag) {
    Handle<JSValue> jsval = JSValue::New(kBoolSize, flag);

    SET_VALUE(jsval.val(), kJSValueOffset, val, bool);

    jsval.val()->SetType(JS_BOOL);
    return Handle<Bool>(jsval);
  }
};

class String : public JSValue {
 public:
  static Handle<String> New(std::u16string data, flag_t flag = 0) {
    Handle<JSValue> jsval = JSValue::New(kSizeTSize + data.size() * kChar16Size, flag);

    SET_VALUE(jsval.val(), kLengthOffset, data.size(), size_t);
    memcpy(PTR(jsval.val(), kStringDataOffset), data.data(), data.size() * kChar16Size);

    jsval.val()->SetType(JS_STRING);
    return Handle<String>(jsval);
  }

  static Handle<String> New(char16_t* data, flag_t flag = 0) {
    size_t size = std::char_traits<char16_t>::length(data);
    Handle<JSValue> jsval = JSValue::New(kSizeTSize + size * kChar16Size, flag);

    SET_VALUE(jsval.val(), kLengthOffset, size, size_t);
    memcpy(PTR(jsval.val(), kStringDataOffset), data, size * kChar16Size);

    jsval.val()->SetType(JS_STRING);
    return Handle<String>(jsval);
  }

  static Handle<String> New(size_t n) {
    Handle<JSValue> jsval = JSValue::New(kSizeTSize + n * kChar16Size);

    SET_VALUE(jsval.val(), kLengthOffset, n, int);

    jsval.val()->SetType(JS_STRING);
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
    static Handle<String> singleton = String::New(u"", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Undefined() {
    static Handle<String> singleton = String::New(u"undefined", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Null() {
    static Handle<String> singleton = String::New(u"null", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> True() {
    static Handle<String> singleton = String::New(u"true", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> False() {
    static Handle<String> singleton = String::New(u"false", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> NaN() {
    static Handle<String> singleton = String::New(u"NaN", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Zero() {
    static Handle<String> singleton = String::New(u"0", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Infinity() {
    static Handle<String> singleton = String::New(u"Infinity", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Prototype() {
    static Handle<String> singleton = String::New(u"prototype", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Constructor() {
    static Handle<String> singleton = String::New(u"constructor", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Length() {
    static Handle<String> singleton = String::New(u"length", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Value() {
    static Handle<String> singleton = String::New(u"value", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Writable() {
    static Handle<String> singleton = String::New(u"writable", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Get() {
    static Handle<String> singleton = String::New(u"get", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Set() {
    static Handle<String> singleton = String::New(u"set", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Arguments() {
    static Handle<String> singleton = String::New(u"arguments", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Enumerable() {
    static Handle<String> singleton = String::New(u"enumerable", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Configurable() {
    static Handle<String> singleton = String::New(u"configurable", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

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
  static Handle<Number> New(double data, flag_t flag = 0) {
    Handle<JSValue> jsval = JSValue::New(kDoubleSize, flag);

    SET_VALUE(jsval.val(), kJSValueOffset, data, double);

    jsval.val()->SetType(JS_NUMBER);
    return Handle<Number>(jsval);
  }

  static Handle<Number> NaN() {
    static Handle<Number> singleton = Number::New(nan(""), GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Number> PositiveInfinity() {
    static Handle<Number> singleton = Number::New(
      std::numeric_limits<double>::infinity(), GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Number> NegativeInfinity() {
    static Handle<Number> singleton = Number::New(
      -std::numeric_limits<double>::infinity(), GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Number> Zero() {
    static Handle<Number> singleton = Number::New(0.0, GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Number> NegativeZero() {
    static Handle<Number> singleton = Number::New(-0.0, GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<Number> One() {
    static Handle<Number> singleton = Number::New(1.0, GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  inline bool IsInfinity() { return isinf(data()); }
  inline bool IsPositiveInfinity() { return data() == std::numeric_limits<double>::infinity(); }
  inline bool IsNegativeInfinity() { return data() == -std::numeric_limits<double>::infinity(); }
  inline bool IsNaN() { return isnan(data()); }

  inline double data() { return READ_VALUE(this, kJSValueOffset, double); }
};

class Error;
void CheckObjectCoercible(Handle<Error>& e, Handle<JSValue> val);

}  // namespace es

#endif  // ES_TYPES_BASE_H