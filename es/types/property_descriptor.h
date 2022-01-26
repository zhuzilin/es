#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_H
#define ES_TYPES_PROPERTY_DESCRIPTOR_H

#include <map>
#include <bitset>

#include <es/types/base.h>

namespace es {

class PropertyDescriptor : public JSValue {
 public:
  static PropertyDescriptor* New() {
    JSValue* jsval = JSValue::New(JS_PROP_DESC, kConfigurableOffset + kBoolSize - kBitmapOffset);
    SET_VALUE(jsval, kBitmapOffset, 0, char);
    SET_VALUE(jsval, kValueOffset, Undefined::Instance(), JSValue*);
    SET_VALUE(jsval, kGetOffset, Undefined::Instance(), JSValue*);
    SET_VALUE(jsval, kSetOffset, Undefined::Instance(), JSValue*);
    SET_VALUE(jsval, kWritableOffset, false, bool);
    SET_VALUE(jsval, kEnumerableOffset, false, bool);
    SET_VALUE(jsval, kConfigurableOffset, false, bool);
    return new (jsval) PropertyDescriptor();
  }

  inline bool IsAccessorDescriptor() {
    return (bitmask() & (GET | SET)) == (GET | SET);
  }

  inline bool IsDataDescriptor() {
    return (bitmask() & (VALUE | WRITABLE)) == (VALUE | WRITABLE);
  }

  inline bool IsGenericDescriptor() {
    return !IsAccessorDescriptor() && !IsDataDescriptor();
  }

  // TODO(zhuzilin) May be check the member variable is initialized?
  inline bool HasValue() { return bitmask() & VALUE; }
  inline JSValue* Value() { return READ_VALUE(this, kValueOffset, JSValue*); }
  inline void SetValue(JSValue* value) {
    SET_VALUE(this, kBitmapOffset, bitmask() | VALUE, char);
    SET_VALUE(this, kValueOffset, value, JSValue*);
  }

  inline bool HasWritable() {return bitmask() & WRITABLE; }
  inline bool Writable() { return READ_VALUE(this, kWritableOffset, bool); }
  inline void SetWritable(bool writable) {
    SET_VALUE(this, kBitmapOffset, bitmask() | WRITABLE, char);
    SET_VALUE(this, kWritableOffset, writable, bool);
  }

  inline bool HasGet() {return bitmask() & GET; }
  inline JSValue* Get() { return READ_VALUE(this, kGetOffset, JSValue*); }
  inline void SetGet(JSValue* getter) {
    SET_VALUE(this, kBitmapOffset, bitmask() | GET, char);
    SET_VALUE(this, kGetOffset, getter, JSValue*);
  }

  inline bool HasSet() { return bitmask() & SET; }
  inline JSValue* Set() { return READ_VALUE(this, kSetOffset, JSValue*); }
  inline void SetSet(JSValue* setter) {
    SET_VALUE(this, kBitmapOffset, bitmask() | SET, char);
    SET_VALUE(this, kSetOffset, setter, JSValue*);
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
    JSValue* value, bool writable, bool enumerable, bool configurable
  ) {
    SetValue(value);
    SetWritable(writable);
    SetEnumerable(enumerable);
    SetConfigurable(configurable);
  }

  inline void SetAccessorDescriptor(
    JSValue* getter, JSValue* setter, bool enumerable, bool configurable
  ) {
    SetGet(getter);
    SetSet(setter);
    SetEnumerable(enumerable);
    SetConfigurable(configurable);
  }

  // Set the value to `this` if `other` has.
  inline void Set(PropertyDescriptor* other) {
    if (other->HasValue())
      SetValue(other->Value());
    if (other->HasWritable())
      SetWritable(other->Writable());
    if (other->HasGet())
      SetGet(other->Get());
    if (other->HasSet())
      SetSet(other->Set());
    if (other->HasConfigurable())
      SetConfigurable(other->Configurable());
    if (other->HasEnumerable())
      SetEnumerable(other->Enumerable());
  }

  char bitmask() { return READ_VALUE(this, kBitmapOffset, char); }
  void SetBitMask(char bitmask) { SET_VALUE(this, kBitmapOffset, bitmask, char); }

  std::string ToString() override { 
    std::string res = "PropertyDescriptor{";
    if (HasValue()) res += "v: " + Value()->ToString() + ", ";
    if (HasWritable()) res += "w: " + log::ToString(Writable()) + ", ";
    if (HasGet()) res += "get: " + Get()->ToString() + ", ";
    if (HasSet()) res += "set: " + Set()->ToString() + ", ";
    if (HasEnumerable()) res += "e: " + log::ToString(Enumerable()) + ", ";
    if (HasConfigurable()) res += "c: " + log::ToString(Configurable());
    res += '}';
    return res;
  }

  std::vector<void*> Pointers() override {
    assert(false);
  }

 private:
  static constexpr size_t kBitmapOffset = kJSValueOffset;
  static constexpr size_t kValueOffset = kBitmapOffset + kCharSize;
  static constexpr size_t kGetOffset = kValueOffset + kPtrSize;
  static constexpr size_t kSetOffset = kGetOffset + kPtrSize;
  static constexpr size_t kWritableOffset = kSetOffset + kPtrSize;
  static constexpr size_t kEnumerableOffset = kWritableOffset + kBoolSize;
  static constexpr size_t kConfigurableOffset = kEnumerableOffset + kBoolSize;

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