#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION
#define ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION

#include <es/types/property_descriptor.h>
#include <es/types/builtin/object_object.h>
#include <es/types/error.h>

namespace es {

bool ToBoolean(JSValue input);

JSValue FromPropertyDescriptor(JSValue desc) {
  if (desc.IsUndefined()) {
    return undefined::New();
  }
  ASSERT(desc.IsPropertyDescriptor());
  JSValue obj = object_object::New();
  if (property_descriptor::IsDataDescriptor(desc)) {
    AddValueProperty(obj, string::Value(), property_descriptor::Value(desc), true, true, true);
    AddValueProperty(obj, string::Writable(), boolean::New(property_descriptor::Writable(desc)), true, true, true);
  } else {
    ASSERT(property_descriptor::IsAccessorDescriptor(desc));
    AddValueProperty(obj, string::Get(), property_descriptor::Get(desc), true, true, true);
    AddValueProperty(obj, string::Set(), property_descriptor::Set(desc), true, true, true);
  }
  AddValueProperty(obj, string::Enumerable(), boolean::New(property_descriptor::Enumerable(desc)), true, true, true);
  AddValueProperty(obj, string::Configurable(), boolean::New(property_descriptor::Configurable(desc)), true, true, true);
  return obj;
}

JSValue ToPropertyDescriptor(JSValue& e, JSValue val) {
  if (!val.IsObject()) {
    e = error::TypeError();
    return property_descriptor::New();
  }
  JSValue obj = val;
  JSValue desc = property_descriptor::New();
  if (HasProperty(obj, string::Enumerable())) {
    JSValue value = Get(e, obj, string::Enumerable());
    property_descriptor::SetEnumerable(desc, ToBoolean(value));
  }
  if (HasProperty(obj, string::Configurable())) {
    JSValue value = Get(e, obj, string::Configurable());
    property_descriptor::SetConfigurable(desc, ToBoolean(value));
  }
  if (HasProperty(obj, string::Value())) {
    JSValue value = Get(e, obj, string::Value());
    property_descriptor::SetValue(desc, value);
  }
  if (HasProperty(obj, string::Writable())) {
    JSValue value = Get(e, obj, string::Writable());
    property_descriptor::SetWritable(desc, ToBoolean(value));
  }
  if (HasProperty(obj, string::Get())) {
    JSValue value = Get(e, obj, string::Get());
    if (!value.IsCallable() && !value.IsUndefined()) {
      e = error::TypeError(u"getter not callable.");
    }
    property_descriptor::SetGet(desc, value);
  }
  if (HasProperty(obj, string::Set())) {
    JSValue value = Get(e, obj, string::Set());
    if (!value.IsCallable() && !value.IsUndefined()) {
      e = error::TypeError(u"setter not callable.");
    }
    property_descriptor::SetSet(desc, value);
  }
  if (property_descriptor::HasSet(desc) || property_descriptor::HasGet(desc)) {
    if (property_descriptor::HasValue(desc) || property_descriptor::HasWritable(desc)) {
      e = error::TypeError(u"cannot have both get/set and value/writable");
      return property_descriptor::New();
    }
  }
  return desc;
}

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION