#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_H
#define ES_TYPES_PROPERTY_DESCRIPTOR_H

#include <map>
#include <bitset>

#include <es/types/base.h>

namespace es {

bool SameValue(Handle<JSValue> x, Handle<JSValue> y);

class PropertyDescriptor : public JSValue {
 public:
  static Handle<PropertyDescriptor> New() {
    Handle<JSValue> jsval = HeapObject::New(kConfigurableOffset + kBoolSize - kBitmapOffset);

    SET_VALUE(jsval.val(), kBitmapOffset, 0, char);
    SET_HANDLE_VALUE(jsval.val(), kValueOffset, Undefined::Instance(), JSValue);
    SET_HANDLE_VALUE(jsval.val(), kGetOffset, Undefined::Instance(), JSValue);
    SET_HANDLE_VALUE(jsval.val(), kSetOffset, Undefined::Instance(), JSValue);
    SET_VALUE(jsval.val(), kWritableOffset, false, bool);
    SET_VALUE(jsval.val(), kEnumerableOffset, false, bool);
    SET_VALUE(jsval.val(), kConfigurableOffset, false, bool);

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
    SET_VALUE(this, kBitmapOffset, bitmask() | VALUE, char);
    SET_HANDLE_VALUE(this, kValueOffset, value, JSValue);
  }

  inline bool HasWritable() {return bitmask() & WRITABLE; }
  inline bool Writable() { return READ_VALUE(this, kWritableOffset, bool); }
  inline void SetWritable(bool writable) {
    SET_VALUE(this, kBitmapOffset, bitmask() | WRITABLE, char);
    SET_VALUE(this, kWritableOffset, writable, bool);
  }

  inline bool HasGet() {return bitmask() & GET; }
  inline Handle<JSValue> Get() { return READ_HANDLE_VALUE(this, kGetOffset, JSValue); }
  inline void SetGet(Handle<JSValue> getter) {
    SET_VALUE(this, kBitmapOffset, bitmask() | GET, char);
    SET_HANDLE_VALUE(this, kGetOffset, getter, JSValue);
  }

  inline bool HasSet() { return bitmask() & SET; }
  inline Handle<JSValue> Set() { return READ_HANDLE_VALUE(this, kSetOffset, JSValue); }
  inline void SetSet(Handle<JSValue> setter) {
    SET_VALUE(this, kBitmapOffset, bitmask() | SET, char);
    SET_HANDLE_VALUE(this, kSetOffset, setter, JSValue);
  }

  inline bool HasEnumerable() { return bitmask() & ENUMERABLE; }
  inline bool Enumerable() { return READ_VALUE(this, kEnumerableOffset, bool); }
  inline void SetEnumerable(bool enumerable) {
    SET_VALUE(this, kBitmapOffset, bitmask() | ENUMERABLE, char);
    SET_VALUE(this, kEnumerableOffset, enumerable, bool);
  }

  inline bool HasConfigurable() { return bitmask() & CONFIGURABLE; }
  inline bool Configurable() { return READ_VALUE(this, kConfigurableOffset, bool); }
  inline void SetConfigurable(bool configurable) {
    SET_VALUE(this, kBitmapOffset, bitmask() | CONFIGURABLE, char);
    SET_VALUE(this, kConfigurableOffset, configurable, bool);
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

  inline void Reset(char new_bitmask, bool enumerable, bool configurable) {
    SET_VALUE(this, kBitmapOffset, new_bitmask, char);
    SET_HANDLE_VALUE(this, kValueOffset, Undefined::Instance(), JSValue);
    SET_HANDLE_VALUE(this, kGetOffset, Undefined::Instance(), JSValue);
    SET_HANDLE_VALUE(this, kSetOffset, Undefined::Instance(), JSValue);
    SET_VALUE(this, kWritableOffset, false, bool);
    SET_VALUE(this, kEnumerableOffset, enumerable, bool);
    SET_VALUE(this, kConfigurableOffset, configurable, bool);
  }

  inline bool IsSameAs(Handle<PropertyDescriptor> other) {
    if (HasValue() && !SameValue(Value(), other.val()->Value()))
      return false;
    if (HasWritable() && (Writable() != other.val()->Writable()))
      return false;
    if (HasGet() && !SameValue(Get(), other.val()->Get()))
      return false;
    if (HasSet() && !SameValue(Set(), other.val()->Set()))
      return false;
    if (HasConfigurable() && (Configurable() != other.val()->Configurable()))
      return false;
    if (HasEnumerable() && (Enumerable() != other.val()->Enumerable()))
      return false;
    return true;
  }

  char bitmask() { return READ_VALUE(this, kBitmapOffset, char); }
  void SetBitMask(char bitmask) { SET_VALUE(this, kBitmapOffset, bitmask, char); }

 public:
  static constexpr size_t kBitmapOffset = kJSValueOffset;
  static constexpr size_t kValueOffset = kBitmapOffset + kCharSize;
  static constexpr size_t kGetOffset = kValueOffset + kPtrSize;
  static constexpr size_t kSetOffset = kGetOffset + kPtrSize;
  static constexpr size_t kWritableOffset = kSetOffset + kPtrSize;
  static constexpr size_t kEnumerableOffset = kWritableOffset + kBoolSize;
  static constexpr size_t kConfigurableOffset = kEnumerableOffset + kBoolSize;

 private:
  enum Field {
    VALUE        = 1 << 0,
    WRITABLE     = 1 << 1,
    GET          = 1 << 2,
    SET          = 1 << 3,
    ENUMERABLE   = 1 << 4,
    CONFIGURABLE = 1 << 5,
  };
};

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_H