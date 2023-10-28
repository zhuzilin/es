#ifndef ES_TYPES_BASE_H
#define ES_TYPES_BASE_H

#include <math.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include <es/gc/heap_object.h>

namespace es {

class JSValue {
 public:
  bool IsCallable();
  bool IsConstructor();

  inline static bool IsUndefined(Type type) { return type == JS_UNDEFINED; }
  inline static bool IsBool(Type type) { return type == JS_BOOL; }
  inline static bool IsString(Type type) { return type == JS_STRING || type == JS_LONG_STRING; }
  inline static bool IsNumber(Type type) { return type == JS_NUMBER; }
  inline static bool IsObject(Type type) { return (type & JS_OBJECT) != 0; }

  inline bool IsUndefined() { return IsUndefined(type()); }
  inline bool IsNull() { return type() == JS_NULL; }
  inline bool IsBool() { return IsBool(type());  }
  inline bool IsString() { return IsString(type()); }
  inline bool IsNumber() { return IsNumber(type()); }
  inline bool IsPrimitive() { return !IsObject(); }

  inline bool IsObject() { return IsObject(type()); }
  inline bool IsPrototype() { return IsNull() || IsObject(); }

  inline bool IsBoolObject() { return type() == OBJ_BOOL; }
  inline bool IsNumberObject() { return type() == OBJ_NUMBER; }
  inline bool IsArrayObject() { return type() == OBJ_ARRAY; }
  inline bool IsRegExpObject() { return type() == OBJ_REGEXP; }
  inline bool IsErrorObject() { return type() == OBJ_ERROR; }
  inline bool IsFunctionObject() { return type() == OBJ_FUNC || type() == OBJ_BIND_FUNC; }
  inline bool IsStringObject() { return type() == OBJ_STRING; }
  inline bool IsDateObject() { return type() == OBJ_DATE; }
  inline bool IsArgumentsObject() { return type() == OBJ_ARGUMENTS; }

  inline bool IsFunctionProto() { return type() == OBJ_FUNC_PROTO; }

  inline bool IsBoolConstructor() { return type() == OBJ_BOOL_CONSTRUCTOR; }
  inline bool IsNumberConstructor() { return type() == OBJ_NUMBER_CONSTRUCTOR; }
  inline bool IsObjectConstructor() { return type() == OBJ_OBJECT_CONSTRUCTOR; }
  inline bool IsRegExpConstructor() { return type() == OBJ_REGEXP_CONSTRUCTOR; }
  inline bool IsStringConstructor() { return type() == OBJ_STRING_CONSTRUCTOR; }

  inline bool IsJSValue() { return (type() & NON_JSVALUE) == 0; }
  inline bool IsLanguageType() { return type() < SPEC_TYPE; }

  inline bool IsReference() { return type() == JS_REF; }
  inline bool IsPropertyDescriptor() { return type() == JS_PROP_DESC; }
  inline bool IsEnvironmentRecord() { return type() == JS_ENV_REC_DECL || type() == JS_ENV_REC_OBJ; }
  inline bool IsLexicalEnvironment() { return type() == JS_LEX_ENV; }

  inline bool IsGetterSetter() { return type() == JS_GET_SET; }

  inline bool IsError() { return type() == ERROR; }

  inline Type stack_type() {
    return static_cast<Type>(reinterpret_cast<uint64_t>(this) & STACK_MASK); }
  inline Type type() {
    return stack_type() ? stack_type() : header_.type_;
  }
  inline void SetType(Type t) { header_.type_ = t; }

  static std::string ToString(JSValue* jsval);

 public:
  static constexpr size_t kJSValueOffset = HeapObject::kHeapObjectOffset;

  struct Header {
    Type type_;
    uint32_t placeholder_;
  };
  Header header_;
};

class Undefined : public JSValue {
 public:
  static Handle<Undefined> Instance() {
    return Handle<Undefined>(reinterpret_cast<Undefined*>(JS_UNDEFINED));
  }
};

class Null : public JSValue {
 public:
  static Handle<Null> Instance() {
    return Handle<Null>(reinterpret_cast<Null*>(JS_NULL));
  }
};

class Bool : public JSValue {
 public:
  static Handle<Bool> True() {
    return Handle<Bool>(reinterpret_cast<Bool*>((1 << STACK_SHIFT) | JS_BOOL));
  }
  static Handle<Bool> False() {
    return Handle<Bool>(reinterpret_cast<Bool*>(JS_BOOL));
  }

  static Handle<Bool> Wrap(bool val) {
    return val ? True() : False();
  }

  inline bool data() { return reinterpret_cast<uint64_t>(this) >> STACK_SHIFT; }
};

inline bool ToArrayIndex(const char16_t*, size_t, double&);
std::u16string ArrayIndexToString(uint32_t index);

class String : public JSValue {
 public:
  static Handle<String> New(const std::u16string& data, flag_t flag = 0) {
    size_t n = data.size();
    double index;
    if (unlikely(ToArrayIndex(data.c_str(), n, index))) {
      return String::New(index, n);
    }
    Handle<String> str = String::Alloc(n, flag);

    memcpy(PTR(str.val(), kStringDataOffset), data.data(), n * kChar16Size);

    return str;
  }

  static Handle<String> New(std::u16string&& data, flag_t flag = 0) {
    size_t n = data.size();
    double index;
    if (unlikely(ToArrayIndex(data.c_str(), n, index))) {
      return String::New(index, n);
    }
    Handle<String> str = String::Alloc(n, flag);

    memcpy(PTR(str.val(), kStringDataOffset), data.data(), n * kChar16Size);

    return str;
  }

  static Handle<String> New(uint32_t index) {
    size_t n = 1;
    uint32_t tmp = index / 10;
    while (tmp) {
      n++;
      tmp /= 10;
    }
    return String::New(index, n);
  }

  bool IsArrayIndex() { return stack_type(); }
  uint32_t Index() {
    ASSERT(IsArrayIndex());
    return reinterpret_cast<uint64_t>(this) >> (STACK_SHIFT + 5);
  }
  std::u16string data() {
    if (IsArrayIndex())
      return ArrayIndexToString(Index());
    return std::u16string(c_str(), size());
  }
  size_t length_slot() {
    ASSERT(!IsArrayIndex());
    return READ_VALUE(this, kLengthOffset, size_t);
  }
  size_t size() {
    if (IsArrayIndex()) {
      return (reinterpret_cast<uint64_t>(this) >> STACK_SHIFT) & 31;
    }
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
    if (IsArrayIndex())
      return Index();
    size_t slot = length_slot();
    if (slot & 1) return slot >> 1;
    size_t hash = U16Hash(data());
    hash = hash << 1 | 1;
    hash = slot | (0x0000FFFF & hash);
    SET_VALUE(this, kLengthOffset, hash, size_t);
    return hash >> 1;
  }

  bool HasHash() { return IsArrayIndex() || (length_slot() & 1); }

  char16_t get(size_t index) {
    ASSERT(index < size());
    if (IsArrayIndex()) {
      uint32_t remains = size() - index - 1;
      uint32_t index = Index();
      while (remains) {
        index /= 10;
      }
      return u'0' + index % 10;
    }
    return c_str()[index];
  }

  static Handle<String> Substr(Handle<String> str, size_t pos, size_t len) {
    ASSERT(pos + len <= str.val()->size());
    if (str.val()->IsArrayIndex()) {
      std::cout << "not implemented yet" << std::endl;
      assert(false);
    }
    double index;
    if (ToArrayIndex(str.val()->c_str() + pos, len, index)) {
      return String::New(index, len);
    }
    Handle<String> substring = String::Alloc(len);
    memcpy(substring.val()->c_str(),
           str.val()->c_str() + pos,
           len * kChar16Size);
    return substring;
  }

  static Handle<String> Concat(Handle<String> a, Handle<String> b) {
    if (a.val()->IsArrayIndex() || b.val()->IsArrayIndex()) {
      return String::New(a.val()->data() + b.val()->data());
    }
    size_t size_a = a.val()->size();
    size_t size_b = b.val()->size();
    Handle<String> str = String::Alloc(size_a + size_b);
    memcpy(str.val()->c_str(),
           a.val()->c_str(),
           size_a * kChar16Size);
    memcpy(str.val()->c_str() + size_a,
           b.val()->c_str(),
           size_b * kChar16Size);
    return str;
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
    return String::New(u"0");
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

  static Handle<String> Eval() {
    static Handle<String> singleton = String::New(u"eval", GCFlag::CONST | GCFlag::SINGLE);
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

  static Handle<String> Caller() {
    static Handle<String> singleton = String::New(u"caller", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

  static Handle<String> Callee() {
    static Handle<String> singleton = String::New(u"callee", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

 private:
  static Handle<String> Alloc(size_t n, flag_t flag = 0) {
    Handle<JSValue> jsval = HeapObject::New(kSizeTSize + n * kChar16Size, flag);

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

  static Handle<String> New(uint32_t index, size_t size) {
    return Handle<String>(reinterpret_cast<String*>(
      (uint64_t(index) << (STACK_SHIFT + 5)) |  // 5 bits for size should be enough
      (size << STACK_SHIFT) |
      JS_STRING));
  }

  char16_t* c_str() {
    ASSERT(!IsArrayIndex());
    return TYPED_PTR(this, kStringDataOffset, char16_t);
  }

  static constexpr size_t kLengthOffset = kJSValueOffset;
  static constexpr size_t kStringDataOffset = kLengthOffset + kSizeTSize;

  static constexpr size_t kLongStringSize = 65536;

  static constexpr std::hash<std::u16string> U16Hash = std::hash<std::u16string>{};
};

inline bool StringEqual(String* a, String* b) {
  if (a->IsArrayIndex() != b->IsArrayIndex())
    return false;
  if (a->IsArrayIndex() && b->IsArrayIndex())
    return a->Index() == b->Index();
  if (a->HasHash() && b->HasHash()) {
    if (a->length_slot() != b->length_slot())
      return false;
  }
  if (a->size() != b->size()) {
    return false;
  }
  size_t size = a->size();
  for (size_t i = 0; i < size; i++) {
    if (a->get(i) != b->get(i))
      return false;
  }
  return true;
}

inline bool StringEqual(Handle<String> a, Handle<String> b) {
  return StringEqual(a.val(), b.val());
}

inline bool StringLessThan(String* a, String* b) {
  if (a->IsArrayIndex() && b->IsArrayIndex()) {
    return a->Index() < b->Index();
  }
  if (a->IsArrayIndex() || b->IsArrayIndex()) {
    return a->data() < b->data();
  }
  size_t size_a = a->size();
  size_t size_b = b->size();
  size_t size = size_a < size_b ? size_a : size_b;

  for (size_t i = 0; i < size; i++) {
    if (a->get(i) != b->get(i))
      return a->get(i) < b->get(i);
  }
  return size_a < size_b;
}

class Number : public JSValue {
 public:
  static Handle<Number> New(double data, flag_t flag = 0) {
    Double2Uint64 tmp;
    tmp.double_ = data;
    if ((tmp.uint64_ & STACK_MASK) == 0) {
      return Handle<Number>(reinterpret_cast<Number*>(tmp.uint64_ | JS_NUMBER));
    }
    Handle<JSValue> jsval = HeapObject::New(kDoubleSize, flag);

    SET_VALUE(jsval.val(), kJSValueOffset, data, double);

    jsval.val()->SetType(JS_NUMBER);
    return Handle<Number>(jsval);
  }

  static Handle<Number> NaN() {
    return Number::New(nan(""));
  }

  static Handle<Number> Infinity() {
    return Number::New(std::numeric_limits<double>::infinity());
  }

  static Handle<Number> NegativeInfinity() {
    return Number::New(-std::numeric_limits<double>::infinity());
  }

  static Handle<Number> Zero() {
    return Number::New(0.0);
  }

  static Handle<Number> NegativeZero() {
    return Number::New(-0.0);
  }

  inline bool IsInfinity() { return isinf(data()); }
  inline bool IsPositiveInfinity() { return data() == std::numeric_limits<double>::infinity(); }
  inline bool IsNegativeInfinity() { return data() == -std::numeric_limits<double>::infinity(); }
  inline bool IsNaN() { return isnan(data()); }

  inline double data() {
    if (stack_type()) {
      Double2Uint64 tmp;
      tmp.uint64_ = reinterpret_cast<uint64_t>(this) & (~STACK_MASK);
      return tmp.double_;
    }
    return READ_VALUE(this, kJSValueOffset, double);
  }
};

class Error;
void CheckObjectCoercible(Handle<Error>& e, Handle<JSValue> val);

}  // namespace es

#endif  // ES_TYPES_BASE_H