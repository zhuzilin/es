#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION
#define ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION

#include <es/types/property_descriptor.h>
#include <es/types/builtin/object_object.h>
#include <es/error.h>

namespace es {

bool ToBoolean(Handle<JSValue> input);

Handle<JSValue> FromPropertyDescriptor(StackPropertyDescriptor desc) {
  if (desc.IsUndefined()) {
    return Undefined::Instance();
  }
  Handle<JSObject> obj = Object::New();
  if (desc.IsDataDescriptor()) {
    AddValueProperty(obj, String::Value(), desc.Value(), true, true, true);
    AddValueProperty(obj, String::Writable(), Bool::Wrap(desc.Writable()), true, true, true);
  } else {
    ASSERT(desc.IsAccessorDescriptor());
    AddValueProperty(obj, String::Get(), desc.Get(), true, true, true);
    AddValueProperty(obj, String::Set(), desc.Set(), true, true, true);
  }
  AddValueProperty(obj, String::Enumerable(), Bool::Wrap(desc.Enumerable()), true, true, true);
  AddValueProperty(obj, String::Configurable(), Bool::Wrap(desc.Configurable()), true, true, true);
  return obj;
}

StackPropertyDescriptor ToPropertyDescriptor(Handle<Error>& e, Handle<JSValue> val) {
  if (unlikely(!val.val()->IsObject())) {
    e = Error::TypeError();
    return StackPropertyDescriptor();
  }
  Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
  StackPropertyDescriptor desc;
  if (HasProperty(obj, String::Enumerable())) {
    Handle<JSValue> value = Get(e, obj, String::Enumerable());
    desc.SetEnumerable(ToBoolean(value));
  }
  if (HasProperty(obj, String::Configurable())) {
    Handle<JSValue> value = Get(e, obj, String::Configurable());
    desc.SetConfigurable(ToBoolean(value));
  }
  if (HasProperty(obj, String::Value())) {
    Handle<JSValue> value = Get(e, obj, String::Value());
    desc.SetValue(value);
  }
  if (HasProperty(obj, String::Writable())) {
    Handle<JSValue> value = Get(e, obj, String::Writable());
    desc.SetWritable(ToBoolean(value));
  }
  if (HasProperty(obj, String::Get())) {
    Handle<JSValue> value = Get(e, obj, String::Get());
    if (!value.val()->IsCallable() && !value.val()->IsUndefined()) {
      e = Error::TypeError(u"getter not callable.");
    }
    desc.SetGet(value);
  }
  if (HasProperty(obj, String::Set())) {
    Handle<JSValue> value = Get(e, obj, String::Set());
    if (!value.val()->IsCallable() && !value.val()->IsUndefined()) {
      e = Error::TypeError(u"setter not callable.");
    }
    desc.SetSet(value);
  }
  if (desc.HasSet() || desc.HasGet()) {
    if (desc.HasValue() || desc.HasWritable()) {
      e = Error::TypeError(u"cannot have both get/set and value/writable");
      return StackPropertyDescriptor();
    }
  }
  return desc;
}

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION