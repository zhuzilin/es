#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_H
#define ES_TYPES_PROPERTY_DESCRIPTOR_H

#include <map>
#include <bitset>

#include <es/types/base.h>

namespace es {

bool SameValue(Handle<JSValue> x, Handle<JSValue> y);

class StackPropertyDescriptor {
 public:
  enum Field {
    VALUE        = 1 << 0,
    WRITABLE     = 1 << 1,
    GET          = 1 << 2,
    SET          = 1 << 3,
    ENUMERABLE   = 1 << 4,
    CONFIGURABLE = 1 << 5,
  };

  StackPropertyDescriptor() :
    bitmask_(0),
    getter_(Undefined::Instance()), setter_(Undefined::Instance()),
    value_(Undefined::Instance()), writable_(false),
    enumerable_(false), configurable_(false),
    is_undefined_(false) {}

  static StackPropertyDescriptor NewDataDescriptor(
    Handle<JSValue> value, bool writable, bool enumerable, bool configurable
  ) {
    StackPropertyDescriptor desc;
    desc.SetValue(value);
    desc.SetWritable(writable);
    desc.SetEnumerable(enumerable);
    desc.SetConfigurable(configurable);
    return desc;
  }

  static StackPropertyDescriptor Undefined() {
    StackPropertyDescriptor desc;
    desc.is_undefined_ = true;
    return desc;
  }

  inline bool IsAccessorDescriptor() {
    return (bitmask() & (GET | SET)) != 0;
  }

  inline bool IsDataDescriptor() {
    return (bitmask() & (VALUE | WRITABLE)) != 0;
  }

  inline bool IsGenericDescriptor() {
    return !IsAccessorDescriptor() && !IsDataDescriptor();
  }

  // TODO(zhuzilin) May be check the member variable is initialized?
  inline bool HasValue() { return bitmask() & VALUE; }
  inline Handle<JSValue> Value() { return value_; }
  inline void SetValue(Handle<JSValue> value) {
    bitmask_ = bitmask_ | VALUE;
    value_ = value;
  }

  inline bool HasWritable() {return bitmask() & WRITABLE; }
  inline bool Writable() { return writable_; }
  inline void SetWritable(bool writable) {
    bitmask_ = bitmask_ | WRITABLE;
    writable_ = writable;
  }

  inline bool HasGet() {return bitmask() & GET; }
  inline Handle<JSValue> Get() { return getter_; }
  inline void SetGet(Handle<JSValue> getter) {
    bitmask_ = bitmask_ | GET;
    getter_ = getter;
  }

  inline bool HasSet() { return bitmask() & SET; }
  inline Handle<JSValue> Set() { return setter_; }
  inline void SetSet(Handle<JSValue> setter) {
    bitmask_ = bitmask_ | SET;
    setter_ = setter;
  }

  inline bool HasEnumerable() { return bitmask() & ENUMERABLE; }
  inline bool Enumerable() { return enumerable_; }
  inline void SetEnumerable(bool enumerable) {
    bitmask_ = bitmask_ | ENUMERABLE;
    enumerable_ = enumerable;
  }

  inline bool HasConfigurable() { return bitmask() & CONFIGURABLE; }
  inline bool Configurable() { return configurable_; }
  inline void SetConfigurable(bool configurable) {
    bitmask_ = bitmask_ | CONFIGURABLE;
    configurable_ = configurable;
  }

  inline void SetDataDescriptor(
    Handle<JSValue> value, bool writable, bool enumerable, bool configurable
  ) {
    SetValue(value);
    SetWritable(writable);
    SetEnumerable(enumerable);
    SetConfigurable(configurable);
  }

  inline void SetAccessorDescriptor(
    Handle<JSValue> getter, Handle<JSValue> setter, bool enumerable, bool configurable
  ) {
    SetGet(getter);
    SetSet(setter);
    SetEnumerable(enumerable);
    SetConfigurable(configurable);
  }

  // Set the value to `this` if `other` has.
  inline void Set(StackPropertyDescriptor other) {
    if (other.HasValue())
      SetValue(other.Value());
    if (other.HasWritable())
      SetWritable(other.Writable());
    if (other.HasGet())
      SetGet(other.Get());
    if (other.HasSet())
      SetSet(other.Set());
    if (other.HasConfigurable())
      SetConfigurable(other.Configurable());
    if (other.HasEnumerable())
      SetEnumerable(other.Enumerable());
  }

  inline void Reset(char new_bitmask, bool enumerable, bool configurable) {
    bitmask_ = new_bitmask;
    value_ = Undefined::Instance();
    getter_ = Undefined::Instance();
    setter_ = Undefined::Instance();
    writable_ = false;
    enumerable_ = enumerable;
    configurable_ = configurable;
  }

  inline bool IsSameAs(StackPropertyDescriptor other) {
    if (HasValue() && !SameValue(Value(), other.Value()))
      return false;
    if (HasWritable() && (Writable() != other.Writable()))
      return false;
    if (HasGet() && !SameValue(Get(), other.Get()))
      return false;
    if (HasSet() && !SameValue(Set(), other.Set()))
      return false;
    if (HasConfigurable() && (Configurable() != other.Configurable()))
      return false;
    if (HasEnumerable() && (Enumerable() != other.Enumerable()))
      return false;
    return true;
  }

  char bitmask() { return bitmask_; }
  void SetBitMask(char bitmask) { bitmask_ = bitmask; }


  bool IsUndefined() { return is_undefined_; }

  std::string ToString() {
    std::string res = "PropertyDescriptor{";
    if (HasValue()) res += "v: " + Value().ToString() + ", ";
    if (HasWritable()) res += "w: " + log::ToString(Writable()) + ", ";
    if (HasGet()) res += "get: " + Get().ToString() + ", ";
    if (HasSet()) res += "set: " + Set().ToString() + ", ";
    if (HasEnumerable()) res += "e: " + log::ToString(Enumerable()) + ", ";
    if (HasConfigurable()) res += "c: " + log::ToString(Configurable());
    res += '}';
    return res;
  }

 private:
  char bitmask_ = 0;
  Handle<JSValue> getter_;
  Handle<JSValue> setter_;
  Handle<JSValue> value_;
  bool writable_;
  bool enumerable_;
  bool configurable_;

  bool is_undefined_;
};

class PropertyDescriptor : public JSValue {
 public:
  static Handle<PropertyDescriptor> New() {
    Handle<JSValue> jsval = HeapObject::New<kDescSize - kJSValueOffset>();

    jsval.val()->SetBitMask(0);

    jsval.val()->SetType(JS_PROP_DESC);
    return Handle<PropertyDescriptor>(jsval);
  }

  static Handle<PropertyDescriptor> NewDataDescriptor(
    Handle<JSValue> value, bool writable, bool enumerable, bool configurable
  ) {
    Handle<JSValue> jsval = HeapObject::New<kDescSize - kJSValueOffset>();

    char bitmask = VALUE | WRITABLE | ENUMERABLE | CONFIGURABLE;
    if (enumerable) bitmask |= ENUMERABLE_V;
    if (configurable) bitmask |= CONFIGURABLE_V;
    jsval.val()->SetBitMask(bitmask);
    SET_HANDLE_VALUE(jsval.val(), kValueOffset, value, JSValue);
    SET_VALUE(jsval.val(), kWritableOffset, writable, bool);

    jsval.val()->SetType(JS_PROP_DESC);
    return Handle<PropertyDescriptor>(jsval);
  }

  inline bool IsAccessorDescriptor() {
    return (bitmask() & (GET | SET)) != 0;
  }

  inline bool IsDataDescriptor() {
    return (bitmask() & (VALUE | WRITABLE)) != 0;
  }

  inline bool IsGenericDescriptor() {
    return !IsAccessorDescriptor() && !IsDataDescriptor();
  }

  // TODO(zhuzilin) May be check the member variable is initialized?
  inline bool HasValue() { return bitmask() & VALUE; }
  inline Handle<JSValue> Value() { return READ_HANDLE_VALUE(this, kValueOffset, JSValue); }
  inline void SetValue(Handle<JSValue> value) {
    SetBitMask(bitmask() | VALUE);
    SET_HANDLE_VALUE(this, kValueOffset, value, JSValue);
  }

  inline bool HasWritable() {return bitmask() & WRITABLE; }
  inline bool Writable() { return READ_VALUE(this, kWritableOffset, bool); }
  inline void SetWritable(bool writable) {
    SetBitMask(bitmask() | WRITABLE);
    SET_VALUE(this, kWritableOffset, writable, bool);
  }

  inline bool HasGet() {return bitmask() & GET; }
  inline Handle<JSValue> Get() { return READ_HANDLE_VALUE(this, kGetOffset, JSValue); }
  inline void SetGet(Handle<JSValue> getter) {
    SetBitMask(bitmask() | GET);
    SET_HANDLE_VALUE(this, kGetOffset, getter, JSValue);
  }

  inline bool HasSet() { return bitmask() & SET; }
  inline Handle<JSValue> Set() { return READ_HANDLE_VALUE(this, kSetOffset, JSValue); }
  inline void SetSet(Handle<JSValue> setter) {
    SetBitMask(bitmask() | SET);
    SET_HANDLE_VALUE(this, kSetOffset, setter, JSValue);
  }

  inline bool HasEnumerable() { return bitmask() & ENUMERABLE; }
  inline bool Enumerable() { return bitmask() & ENUMERABLE_V; }
  inline void SetEnumerable(bool enumerable) {
    SetBitMask(bitmask() | ENUMERABLE | (enumerable ? ENUMERABLE_V : 0));
  }

  inline bool HasConfigurable() { return bitmask() & CONFIGURABLE; }
  inline bool Configurable() { return bitmask() & CONFIGURABLE_V; }
  inline void SetConfigurable(bool configurable) {
    SetBitMask(bitmask() | CONFIGURABLE | (configurable ? CONFIGURABLE_V : 0));
  }

  inline void SetDataDescriptor(
    Handle<JSValue> value, bool writable, bool enumerable, bool configurable
  ) {
    SetValue(value);
    SetWritable(writable);
    SetEnumerable(enumerable);
    SetConfigurable(configurable);
  }

  inline void SetAccessorDescriptor(
    Handle<JSValue> getter, Handle<JSValue> setter, bool enumerable, bool configurable
  ) {
    SetGet(getter);
    SetSet(setter);
    SetEnumerable(enumerable);
    SetConfigurable(configurable);
  }

  // Set the value to `this` if `other` has.
  inline void Set(Handle<PropertyDescriptor> other) {
    if (other.val()->HasValue())
      SetValue(other.val()->Value());
    if (other.val()->HasWritable())
      SetWritable(other.val()->Writable());
    if (other.val()->HasGet())
      SetGet(other.val()->Get());
    if (other.val()->HasSet())
      SetSet(other.val()->Set());
    if (other.val()->HasConfigurable())
      SetConfigurable(other.val()->Configurable());
    if (other.val()->HasEnumerable())
      SetEnumerable(other.val()->Enumerable());
  }

 public:
  static constexpr size_t kGetOffset = kJSValueOffset;
  static constexpr size_t kSetOffset = kGetOffset + kPtrSize;

  static constexpr size_t kValueOffset = kJSValueOffset;
  static constexpr size_t kWritableOffset = kValueOffset + kPtrSize;

  static constexpr size_t kDescSize = kSetOffset + kPtrSize;

 private:
  enum Field : uint8_t {
    VALUE          = 1 << 0,
    WRITABLE       = 1 << 1,
    GET            = 1 << 2,
    SET            = 1 << 3,
    ENUMERABLE     = 1 << 4,
    CONFIGURABLE   = 1 << 5,
    ENUMERABLE_V   = 1 << 6,
    CONFIGURABLE_V = 1 << 7,
  };
};

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_H