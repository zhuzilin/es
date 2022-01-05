#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_H
#define ES_TYPES_PROPERTY_DESCRIPTOR_H

#include <map>

#include <es/types/base.h>

namespace es {

class PropertyDescriptor : public JSValue {
 public:
  enum Field {
    VALUE        = 1,
    WRITABLE     = 1 << 2,
    GET          = 1 << 3,
    SET          = 1 << 4,
    ENUMERABLE   = 1 << 5,
    CONFIGURABLE = 1 << 6,
  };

  PropertyDescriptor(
  ) : JSValue(JS_PROP_DESC), bitmask_(0) {}

  inline bool IsAccessorDescriptor() {
    return bitmask_ & (GET | SET);
  }

  inline bool IsDataDescriptor() {
    return bitmask_ & (VALUE | WRITABLE);
  }

  inline bool IsGenericDescriptor() {
    return !IsAccessorDescriptor() && !IsDataDescriptor();
  }

  // TODO(zhuzilin) May be check the member variable is initialized?
  inline bool HasValue() { return bitmask_ | VALUE; }
  inline JSValue* Value() { return value_; }
  inline void SetValue(JSValue* value) { bitmask_ |= VALUE; value_ = value; }

  inline bool HasWritable() { return bitmask_ | WRITABLE; }
  inline bool Writable() { return writable_; }
  inline void SetWritable(bool writable) { bitmask_ |= WRITABLE; writable_ = writable; }

  inline bool HasGet() { return bitmask_ | GET; }
  inline JSValue* Get() { return getter_; }
  inline void SetGet(JSValue* getter) { bitmask_ |= GET; getter_ = getter; }

  inline bool HasSet() { return bitmask_ | SET; }
  inline JSValue* Set() { return setter_; }
  inline void SetSet(JSValue* setter) { bitmask_ |= SET; setter_ = setter; }

  inline bool HasEnumerable() { return bitmask_ | ENUMERABLE; }
  inline bool Enumerable() { return enumerable_; }
  inline void SetEnumerable(bool enumerable) { bitmask_ |= ENUMERABLE; enumerable_ = enumerable; }

  inline bool HasConfigurable() { return bitmask_ | CONFIGURABLE; }
  inline bool Configurable() { return configurable_; }
  inline void SetConfigurable(bool configurable) { bitmask_ |= CONFIGURABLE; configurable_ = configurable; }

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

  inline void Set(PropertyDescriptor* other) {
    // TODO(zhuzilin)
  }

  char bitmask() { return bitmask_; }

 private:
  char bitmask_;

  JSValue* value_;
  bool writable_;
  JSValue* getter_;
  JSValue* setter_;
  bool enumerable_;
  bool configurable_;
};

class PropertyIdentifier : public JSValue {
 public:
  PropertyIdentifier(std::u16string_view name, PropertyDescriptor* desciptor) :
    JSValue(JS_PROP_IDEN), name_(name), desciptor_(desciptor) {}

 private:
  std::u16string_view name_;
  PropertyDescriptor* desciptor_;
};

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_H