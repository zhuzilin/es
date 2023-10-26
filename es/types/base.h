#ifndef ES_TYPES_BASE_H
#define ES_TYPES_BASE_H

#include <math.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include <es/gc/heap_object.h>

namespace es {

namespace error {

enum ErrorType {
  E_OK = 0,
  E_EVAL,
  E_RANGE,
  E_REFERENCE,
  E_SYNTAX,
  E_TYPE,
  E_URI,
  E_NATIVE,
};

}

class JSValue {
 public:
  JSValue(Type type = JS_UNINIT) : header_(type) {}

  bool IsCallable();
  bool IsConstructor();

  inline bool IsUndefined() { return type() == JS_UNDEFINED; }
  inline bool IsNull() { return type() == JS_NULL; }
  inline bool IsBool() { return type() == JS_BOOL; }
  inline bool IsString() { return type() == JS_STRING || type() == JS_LONG_STRING; }
  inline bool IsNumber() { return type() == JS_NUMBER; }
  inline bool IsPrimitive() { return !IsObject(); }

  inline bool IsObject() { return (type() & JS_OBJECT) != 0; }
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
  inline bool IsLanguageType() { return !IsSpecificationType(); }
  inline bool IsSpecificationType() { return type() & SPEC_TYPE; }

  inline bool IsReference() { return type() == JS_REF; }
  inline bool IsPropertyDescriptor() { return type() == JS_PROP_DESC; }
  inline bool IsEnvironmentRecord() { return type() == JS_ENV_REC_DECL || type() == JS_ENV_REC_OBJ; }
  inline bool IsLexicalEnvironment() { return type() == JS_LEX_ENV; }

  bool IsDeclarativeEnv() { return type() == JS_ENV_REC_DECL; }
  bool IsObjectEnv() { return type() == JS_ENV_REC_OBJ; }

  inline bool IsGetterSetter() { return type() == JS_GET_SET; }

  inline bool IsError() { return type() == ERROR; }

  inline Type type() { return header_.type_; }
  inline void SetType(Type t) { header_.type_ = t; }

  static std::string ToString(Type type);
  static std::string ToString(JSValue jsval);

  Handle<HeapObject>& handle() { return body_.handle_; }

  bool operator==(JSValue other) {
    return header_.type_ == other.header_.type_ &&
           header_.placeholder_.uint32_ == other.header_.placeholder_.uint32_ &&
           body_.uint64_ == other.body_.uint64_;
  }

  static std::vector<JSValue> RelevantValues(JSValue jsval);

  // can only store things that won't be modified.
  struct Header {
    Type type_;
    union Placeholder {
      uint32_t uint32_;
      // string
      uint32_t size_;
      // binding
      struct BindingHeader {
        bool can_delete_;
        bool is_mutable_;
      } binding_header_;
      // reference
      struct ReferenceHeader {
        bool strict_reference_;
        bool has_primitive_base_;
        bool is_property_reference_;
        bool is_unresolvable_reference_;
      } reference_header_;
      // env_rec
      bool provide_this_;
      // error
      error::ErrorType error_type_;
      Placeholder() { uint32_ = 0; }
    } placeholder_;

    Header(Type type) : type_(type) {}
  } header_;
  // 8 bit
  union Body {
    uint64_t uint64_;
    Handle<HeapObject> handle_;
    bool b_;
    double num_;
    char16_t str_[32];
    Body() { uint64_ = 0; }
  } body_;
};

namespace undefined {
inline JSValue New() { return JSValue(JS_UNDEFINED); }
}  // namespace undefined

namespace null {
inline JSValue New() { return JSValue(JS_NULL); }
}  // namespace null

namespace boolean {
inline JSValue New(bool val) {
  JSValue jsval(JS_BOOL);
  jsval.body_.b_ = val;
  return jsval;
}
inline JSValue True() { return New(true); }
inline JSValue False() { return New(false); }
inline bool data(JSValue jsval) { return jsval.body_.b_; }
}  // namespace boolean

// TODO: calculate hash for short string
namespace string {
constexpr size_t kStringDataOffset = 0;
inline JSValue New(size_t n, flag_t flag = 0) {
  JSValue jsval;
  jsval.header_.placeholder_.size_ = n;
  if (n <= 32) {
    jsval.SetType(JS_STRING);
  } else {
    jsval.handle() = HeapObject::New(n * kChar16Size, flag);
    jsval.SetType(JS_LONG_STRING);
  }
  return jsval;
}

inline JSValue New(const std::u16string& data, flag_t flag = 0) {
  JSValue jsval = New(data.size(), flag);
  if (jsval.type() == JS_STRING) {
    memcpy(jsval.body_.str_, data.data(), data.size() * kChar16Size);
  } else {
    memcpy(PTR(jsval.handle().val(), kStringDataOffset), data.data(), data.size() * kChar16Size);
  }
  return jsval;
}

inline JSValue New(const std::u16string&& data, flag_t flag = 0) {
  JSValue jsval = New(data.size(), flag);
  if (jsval.type() == JS_STRING) {
    memcpy(jsval.body_.str_, data.data(), data.size() * kChar16Size);
  } else {
    memcpy(PTR(jsval.handle().val(), kStringDataOffset), data.data(), data.size() * kChar16Size);
  }
  return jsval;
}

inline char16_t* c_str(JSValue jsval) {
  ASSERT(jsval.IsString());
  if (jsval.type() == JS_STRING) {
    return jsval.body_.str_;
  } else {
    return TYPED_PTR(jsval.handle().val(), kStringDataOffset, char16_t);
  }
}

inline size_t size(JSValue jsval) {
  ASSERT(jsval.IsString());
  return jsval.header_.placeholder_.size_;
}

inline std::u16string data(JSValue jsval) {
  return std::u16string(c_str(jsval), size(jsval));
}

inline uint64_t hash(JSValue jsval) {
  ASSERT(jsval.IsString());
  return std::hash<std::u16string_view>{}(string::data(jsval));
  // if (jsval.type() == JS_STRING) {
  //   return jsval.body_.uint64_;
  // } else {
  //   return std::hash<std::u16string_view>{}(string::data(jsval));
  // }
}

inline JSValue Substr(JSValue str, size_t pos, size_t len) {
  ASSERT(str.IsString());
  JSValue substring = New(len);
  memcpy(c_str(substring), c_str(str) + pos, len * kChar16Size);
  return substring;
}

inline JSValue Empty() {
  static JSValue singleton = New(u"", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Undefined() {
  static JSValue singleton = New(u"undefined", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

inline JSValue Null() {
  static JSValue singleton = New(u"null", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue True() {
  static JSValue singleton = New(u"true", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue False() {
  static JSValue singleton = New(u"false", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue NaN() {
  static JSValue singleton = New(u"NaN", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Zero() {
  static JSValue singleton = New(u"0", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Infinity() {
  static JSValue singleton = New(u"Infinity", GCFlag::CONST | GCFlag::SINGLE);
    return singleton;
  }

inline JSValue NegativeInfinity() {
  static JSValue singleton = New(u"-Infinity", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Prototype() {
  static JSValue singleton = New(u"prototype", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Constructor() {
  static JSValue singleton = New(u"constructor", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Length() {
  static JSValue singleton = New(u"length", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Value() {
  static JSValue singleton = New(u"value", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Writable() {
  static JSValue singleton = New(u"writable", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Get() {
  static JSValue singleton = New(u"get", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Set() {
  static JSValue singleton = New(u"set", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Arguments() {
  static JSValue singleton = New(u"arguments", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Enumerable() {
  static JSValue singleton = New(u"enumerable", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

inline JSValue Configurable() {
  static JSValue singleton = New(u"configurable", GCFlag::CONST | GCFlag::SINGLE);
  return singleton;
}

}  // namespace string

namespace number {
inline JSValue New(double data) {
  JSValue jsval(JS_NUMBER);
  jsval.body_.num_ = data;
  return jsval;
}

inline JSValue NaN() { return New(nan("")); }

inline JSValue Infinity() {
  return New(std::numeric_limits<double>::infinity());
}

inline JSValue NegativeInfinity() {
  return New(-std::numeric_limits<double>::infinity());
}

inline JSValue Zero() {
  return New(0.0);
}

inline JSValue NegativeZero() {
  return New(-0.0);
}

inline JSValue One() {
  return New(1.0);
}

inline double data(JSValue jsval) { return jsval.body_.num_; }
inline bool IsInfinity(JSValue jsval) { return isinf(data(jsval)); }
inline bool IsPositiveInfinity(JSValue jsval) { return data(jsval) == std::numeric_limits<double>::infinity(); }
inline bool IsNegativeInfinity(JSValue jsval) { return data(jsval) == -std::numeric_limits<double>::infinity(); }
inline bool IsNaN(JSValue jsval) { return isnan(data(jsval)); }

}  // namespace number

class Error;
void CheckObjectCoercible(JSValue& e, JSValue val);

}  // namespace es

#endif  // ES_TYPES_BASE_H