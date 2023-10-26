#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_H
#define ES_TYPES_PROPERTY_DESCRIPTOR_H

#include <map>
#include <bitset>

#include <es/types/base.h>

namespace es {

bool SameValue(JSValue x, JSValue y);

namespace property_descriptor {

constexpr size_t kValueOffset = 0;
constexpr size_t kGetOffset = kValueOffset + sizeof(JSValue);
constexpr size_t kSetOffset = kGetOffset + sizeof(JSValue);
constexpr size_t kWritableOffset = kSetOffset + sizeof(JSValue);
constexpr size_t kEnumerableOffset = kWritableOffset + kBoolSize;
constexpr size_t kConfigurableOffset = kEnumerableOffset + kBoolSize;
constexpr size_t kBitmapOffset = kConfigurableOffset + kBoolSize;
constexpr size_t kPropertyDescriptorSize = kBitmapOffset + kCharSize;

enum Field {
  VALUE        = 1 << 0,
  WRITABLE     = 1 << 1,
  GET          = 1 << 2,
  SET          = 1 << 3,
  ENUMERABLE   = 1 << 4,
  CONFIGURABLE = 1 << 5,
};

inline JSValue New() {
  JSValue jsval;
  jsval.handle() = HeapObject::New(kPropertyDescriptorSize);

  SET_JSVALUE(jsval.handle().val(), kValueOffset, undefined::New());
  SET_JSVALUE(jsval.handle().val(), kGetOffset, undefined::New());
  SET_JSVALUE(jsval.handle().val(), kSetOffset, undefined::New());
  SET_VALUE(jsval.handle().val(), kBitmapOffset, 0, char);
  SET_VALUE(jsval.handle().val(), kWritableOffset, false, bool);
  SET_VALUE(jsval.handle().val(), kEnumerableOffset, false, bool);
  SET_VALUE(jsval.handle().val(), kConfigurableOffset, false, bool);

  jsval.SetType(JS_PROP_DESC);
  return jsval;
}

inline char bitmask(JSValue desc) {
  return READ_VALUE(desc.handle().val(), kBitmapOffset, char);
}
inline void SetBitMask(JSValue& desc, char bitmask) {
  SET_VALUE(desc.handle().val(), kBitmapOffset, bitmask, char);
}

inline bool IsAccessorDescriptor(JSValue desc) {
  return (bitmask(desc) & (GET | SET)) != 0;
}

inline bool IsDataDescriptor(JSValue desc) {
  return (bitmask(desc) & (VALUE | WRITABLE)) != 0;
}

inline bool IsGenericDescriptor(JSValue desc) {
  return !IsAccessorDescriptor(desc) && !IsDataDescriptor(desc);
}

// TODO(zhuzilin) May be check the member variable is initialized?
inline bool HasValue(JSValue desc) { return bitmask(desc) & VALUE; }
inline JSValue Value(JSValue desc) {
  return GET_JSVALUE(desc.handle().val(), kValueOffset);
}
inline void SetValue(JSValue& desc, JSValue value) {
  SetBitMask(desc, bitmask(desc) | VALUE);
  SET_JSVALUE(desc.handle().val(), kValueOffset, value);
}

inline bool HasWritable(JSValue desc) {return bitmask(desc) & WRITABLE; }
inline bool Writable(JSValue desc) {
  return READ_VALUE(desc.handle().val(), kWritableOffset, bool);
}
inline void SetWritable(JSValue& desc, bool writable) {
  SetBitMask(desc, bitmask(desc) | WRITABLE);
  SET_VALUE(desc.handle().val(), kWritableOffset, writable, bool);
}

inline bool HasGet(JSValue desc) {return bitmask(desc) & GET; }
inline JSValue Get(JSValue desc) { return GET_JSVALUE(desc.handle().val(), kGetOffset); }
inline void SetGet(JSValue& desc, JSValue getter) {
  SetBitMask(desc, bitmask(desc) | GET);
  SET_JSVALUE(desc.handle().val(), kGetOffset, getter);
}

inline bool HasSet(JSValue desc) { return bitmask(desc) & SET; }
inline JSValue Set(JSValue desc) { return GET_JSVALUE(desc.handle().val(), kSetOffset); }
inline void SetSet(JSValue& desc, JSValue setter) {
  SetBitMask(desc, bitmask(desc) | SET);
  SET_JSVALUE(desc.handle().val(), kSetOffset, setter);
}

inline bool HasEnumerable(JSValue desc) { return bitmask(desc) & ENUMERABLE; }
inline bool Enumerable(JSValue desc) {
  return READ_VALUE(desc.handle().val(), kEnumerableOffset, bool);
}
inline void SetEnumerable(JSValue& desc, bool enumerable) {
  SetBitMask(desc, bitmask(desc) | ENUMERABLE);
  SET_VALUE(desc.handle().val(), kEnumerableOffset, enumerable, bool);
}

inline bool HasConfigurable(JSValue desc) { return bitmask(desc) & CONFIGURABLE; }
inline bool Configurable(JSValue desc) {
  return READ_VALUE(desc.handle().val(), kConfigurableOffset, bool);
}
inline void SetConfigurable(JSValue& desc, bool configurable) {
  SetBitMask(desc, bitmask(desc) | CONFIGURABLE);
  SET_VALUE(desc.handle().val(), kConfigurableOffset, configurable, bool);
}

inline void SetDataDescriptor(
  JSValue& desc, JSValue value, bool writable, bool enumerable, bool configurable
) {
  SetValue(desc, value);
  SetWritable(desc, writable);
  SetEnumerable(desc, enumerable);
  SetConfigurable(desc, configurable);
}

inline void SetAccessorDescriptor(
  JSValue& desc, JSValue getter, JSValue setter, bool enumerable, bool configurable
) {
  SetGet(desc, getter);
  SetSet(desc, setter);
  SetEnumerable(desc, enumerable);
  SetConfigurable(desc, configurable);
}

// Set the value to `this` if `other` has.
inline void Set(JSValue& desc, JSValue other) {
  if (HasValue(other))
    SetValue(desc, Value(other));
  if (HasWritable(other))
    SetWritable(desc, Writable(other));
  if (HasGet(other))
    SetGet(desc, Get(other));
  if (HasSet(other))
    SetSet(desc, Set(other));
  if (HasConfigurable(other))
    SetConfigurable(desc, Configurable(other));
  if (HasEnumerable(other))
    SetEnumerable(desc, Enumerable(other));
}

inline void Reset(JSValue& desc, char new_bitmask, bool enumerable, bool configurable) {
  SetBitMask(desc, new_bitmask);
  SetWritable(desc, false);
  SetEnumerable(desc, enumerable);
  SetConfigurable(desc, configurable);
  SetValue(desc, undefined::New());
  SetGet(desc, undefined::New());
  SetSet(desc, undefined::New());
}

inline bool IsSameAs(JSValue desc, JSValue other) {
  if (HasValue(desc) && !SameValue(Value(desc), Value(other)))
    return false;
  if (HasWritable(desc) && (Writable(desc) != Writable(other)))
    return false;
  if (HasGet(desc) && !SameValue(Get(desc), Get(other)))
    return false;
  if (HasSet(desc) && !SameValue(Set(desc), Set(other)))
    return false;
  if (HasConfigurable(desc) && (Configurable(desc) != Configurable(other)))
    return false;
  if (HasEnumerable(desc) && (Enumerable(desc) != Enumerable(other)))
    return false;
  return true;
}

}  // namespace property_descriptor
}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_H