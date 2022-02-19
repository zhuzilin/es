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
  static Handle<String> New(const std::u16string& data, flag_t flag = 0) {
    size_t n = data.size();
    Handle<String> str = String::New(n, flag);

    memcpy(PTR(str.val(), kStringDataOffset), data.data(), n * kChar16Size);

    return str;
  }

  static Handle<String> New(std::u16string&& data, flag_t flag = 0) {
    size_t n = data.size();
    Handle<String> str = String::New(n, flag);

    memcpy(PTR(str.val(), kStringDataOffset), data.data(), n * kChar16Size);

    return str;
  }

  static Handle<String> New(size_t n, flag_t flag = 0) {
    Handle<JSValue> jsval = JSValue::New(kSizeTSize + n * kChar16Size, flag);

    if (n < kLongStringSize) {
      // The last digit is for decide whether hash is calculated.
      SET_VALUE(jsval.val(), kLengthOffset, n << 16, size_t);
      jsval.val()->SetType(JS_STRING);
    } else {
      SET_VALUE(jsval.val(), kLengthOffset, n << 1, size_t);
      jsval.val()->SetType(JS_LONG_STRING);
    }
    return Handle<String>(jsval);
  }

  std::u16string data() { return std::u16string(c_str(), size()); }
  char16_t* c_str() { return TYPED_PTR(this, kStringDataOffset, char16_t); }
  size_t length_slot() { return READ_VALUE(this, kLengthOffset, size_t); }
  size_t size() {
    size_t slot = length_slot();
    switch (type()) {
      case JS_STRING:
        return slot >> 16;
      case JS_LONG_STRING:
        return slot >> 1;
      default:
        assert(false);
    }
  }
  size_t Hash() {
    size_t slot = length_slot();
    if (slot & 1) return slot >> 1;
    size_t hash = U16Hash(data());
    hash = hash << 1 | 1;
    hash = slot | (0x0000FFFF & hash);
    SET_VALUE(this, kLengthOffset, hash, size_t);
    return hash >> 1;
  }

  bool HasHash() { return length_slot() & 1; }

  char16_t& operator [](int index) {
    return c_str()[index];
  }

  static Handle<String> Substr(Handle<String> str, size_t pos, size_t len) {
    Handle<String> substring = String::New(len);
    memcpy(
      PTR(substring.val(), kStringDataOffset),
      str.val()->c_str() + pos,
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

  static Handle<String> NegativeInfinity() {
    static Handle<String> singleton = String::New(u"-Infinity", GCFlag::CONST | GCFlag::SINGLE);
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

  static constexpr size_t kLongStringSize = 65536;

  static constexpr std::hash<std::u16string> U16Hash = std::hash<std::u16string>{};
};

inline bool operator ==(String& a, String& b) {
  if (a.HasHash() && b.HasHash()) {
    if (a.length_slot() != b.length_slot())
      return false;
  } else if (a.size() != b.size()) {
    return false;
  }
  size_t size = a.size();
  for (size_t i = 0; i < size; i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

inline bool operator !=(String& a, String& b) {
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

  static Handle<Number> Infinity() {
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