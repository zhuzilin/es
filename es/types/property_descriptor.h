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
  std::cout << "enter desc" << std::endl;
  jsval.handle() = HeapObject::New(3 * sizeof(JSValue));

  SET_JSVALUE(jsval.handle().val(), kValueOffset, undefined::New());
  SET_JSVALUE(jsval.handle().val(), kGetOffset, undefined::New());
  SET_JSVALUE(jsval.handle().val(), kSetOffset, undefined::New());

  jsval.SetType(JS_PROP_DESC);
  jsval.header_.placeholder_.pb_header_.bitmask_ = 0;
  jsval.header_.placeholder_.pb_header_.writable_ = false;
  jsval.header_.placeholder_.pb_header_.enumerable_ = false;
  jsval.header_.placeholder_.pb_header_.configurable_ = false;
  return jsval;
}

inline char bitmask(JSValue desc) {
  return desc.header_.placeholder_.pb_header_.bitmask_;
}
inline void SetBitMask(JSValue& desc, char bitmask) {
  desc.header_.placeholder_.pb_header_.bitmask_ = bitmask;
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
inline JSValue Value(JSValue desc) { return GET_JSVALUE(desc.handle().val(), kValueOffset); }
inline void SetValue(JSValue& desc, JSValue value) {
  SetBitMask(desc, bitmask(desc) | VALUE);
  SET_JSVALUE(desc.handle().val(), kValueOffset, value);
}

inline bool HasWritable(JSValue desc) {return bitmask(desc) & WRITABLE; }
inline bool Writable(JSValue desc) { return desc.header_.placeholder_.pb_header_.writable_; }
inline void SetWritable(JSValue& desc, bool writable) {
  SetBitMask(desc, bitmask(desc) | WRITABLE);
  desc.header_.placeholder_.pb_header_.writable_ = writable;
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
inline bool Enumerable(JSValue desc) { return desc.header_.placeholder_.pb_header_.enumerable_; }
inline void SetEnumerable(JSValue& desc, bool enumerable) {
  SetBitMask(desc, bitmask(desc) | ENUMERABLE);
  desc.header_.placeholder_.pb_header_.enumerable_ = enumerable;
}

inline bool HasConfigurable(JSValue desc) { return bitmask(desc) & CONFIGURABLE; }
inline bool Configurable(JSValue desc) { return desc.header_.placeholder_.pb_header_.configurable_; }
inline void SetConfigurable(JSValue& desc, bool configurable) {
  SetBitMask(desc, bitmask(desc) | CONFIGURABLE);
  desc.header_.placeholder_.pb_header_.configurable_ = configurable;
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
  desc.header_.placeholder_.pb_header_.bitmask_ = new_bitmask;
  desc.header_.placeholder_.pb_header_.writable_ = false;
  desc.header_.placeholder_.pb_header_.enumerable_ = enumerable;
  desc.header_.placeholder_.pb_header_.configurable_ = configurable;
  SET_JSVALUE(desc.handle().val(), kValueOffset, undefined::New());
  SET_JSVALUE(desc.handle().val(), kGetOffset, undefined::New());
  SET_JSVALUE(desc.handle().val(), kSetOffset, undefined::New());
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