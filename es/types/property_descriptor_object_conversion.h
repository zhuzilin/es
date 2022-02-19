#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION
#define ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION

#include <es/types/property_descriptor.h>
#include <es/types/builtin/object_object.h>
#include <es/error.h>

namespace es {

bool ToBoolean(Handle<JSValue> input);

Handle<JSValue> FromPropertyDescriptor(Handle<JSValue> value) {
  if (value.val()->IsUndefined()) {
    return Undefined::Instance();
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
  Handle<JSObject> obj = Object::New();
  if (desc.val()->IsDataDescriptor()) {
    AddValueProperty(obj, String::Value(), desc.val()->Value(), true, true, true);
    AddValueProperty(obj, String::Writable(), Bool::Wrap(desc.val()->Writable()), true, true, true);
  } else {
    ASSERT(desc.val()->IsAccessorDescriptor());
    AddValueProperty(obj, String::Get(), desc.val()->Get(), true, true, true);
    AddValueProperty(obj, String::Set(), desc.val()->Set(), true, true, true);
  }
  AddValueProperty(obj, String::Enumerable(), Bool::Wrap(desc.val()->Enumerable()), true, true, true);
  AddValueProperty(obj, String::Configurable(), Bool::Wrap(desc.val()->Configurable()), true, true, true);
  return obj;
}

Handle<PropertyDescriptor> ToPropertyDescriptor(Handle<Error>& e, Handle<JSValue> val) {
  if (!val.val()->IsObject()) {
    e = Error::TypeError();
    return Handle<PropertyDescriptor>();
  }
  Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
  Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
  if (HasProperty(obj, String::Enumerable())) {
    Handle<JSValue> value = Get(e, obj, String::Enumerable());
    desc.val()->SetEnumerable(ToBoolean(value));
  }
  if (HasProperty(obj, String::Configurable())) {
    Handle<JSValue> value = Get(e, obj, String::Configurable());
    desc.val()->SetConfigurable(ToBoolean(value));
  }
  if (HasProperty(obj, String::Value())) {
    Handle<JSValue> value = Get(e, obj, String::Value());
    desc.val()->SetValue(value);
  }
  if (HasProperty(obj, String::Writable())) {
    Handle<JSValue> value = Get(e, obj, String::Writable());
    desc.val()->SetWritable(ToBoolean(value));
  }
  if (HasProperty(obj, String::Get())) {
    Handle<JSValue> value = Get(e, obj, String::Get());
    if (!value.val()->IsCallable() && !value.val()->IsUndefined()) {
      e = Error::TypeError(u"getter not callable.");
    }
    desc.val()->SetGet(value);
  }
  if (HasProperty(obj, String::Set())) {
    Handle<JSValue> value = Get(e, obj, String::Set());
    if (!value.val()->IsCallable() && !value.val()->IsUndefined()) {
      e = Error::TypeError(u"setter not callable.");
    }
    desc.val()->SetSet(value);
  }
  if (desc.val()->HasSet() || desc.val()->HasGet()) {
    if (desc.val()->HasValue() || desc.val()->HasWritable()) {
      e = Error::TypeError(u"cannot have both get/set and value/writable");
      return Handle<PropertyDescriptor>();
    }
  }
  return desc;
}

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION