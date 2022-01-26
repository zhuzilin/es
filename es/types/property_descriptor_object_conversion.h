#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION
#define ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION

#include <es/types/property_descriptor.h>
#include <es/types/builtin/object_object.h>
#include <es/error.h>

namespace es {

bool ToBoolean(JSValue* input);

JSValue* FromPropertyDescriptor(JSValue* value) {
  if (value->IsUndefined()) {
    return Undefined::Instance();
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  JSObject* obj = Object::New();
  if (desc->IsDataDescriptor()) {
    obj->AddValueProperty(String::Value(), desc->Value(), true, true, true);
    obj->AddValueProperty(String::Writable(), Bool::Wrap(desc->Writable()), true, true, true);
  } else {
    assert(desc->IsAccessorDescriptor());
    obj->AddValueProperty(String::Get(), desc->Get(), true, true, true);
    obj->AddValueProperty(String::Set(), desc->Set(), true, true, true);
  }
  obj->AddValueProperty(String::Enumerable(), Bool::Wrap(desc->Enumerable()), true, true, true);
  obj->AddValueProperty(String::Configurable(), Bool::Wrap(desc->Configurable()), true, true, true);
  return obj;
}

PropertyDescriptor* ToPropertyDescriptor(Error* e, JSValue* val) {
  if (!val->IsObject()) {
    *e = *Error::TypeError();
    return nullptr;
  }
  JSObject* obj = static_cast<JSObject*>(val);
  PropertyDescriptor* desc = PropertyDescriptor::New();
  if (obj->HasProperty(String::Enumerable())) {
    JSValue* value = obj->Get(e, String::Enumerable());
    desc->SetEnumerable(ToBoolean(value));
  }
  if (obj->HasProperty(String::Configurable())) {
    JSValue* value = obj->Get(e, String::Configurable());
    desc->SetConfigurable(ToBoolean(value));
  }
  if (obj->HasProperty(String::Value())) {
    JSValue* value = obj->Get(e, String::Value());
    desc->SetValue(value);
  }
  if (obj->HasProperty(String::Writable())) {
    JSValue* value = obj->Get(e, String::Writable());
    desc->SetWritable(ToBoolean(value));
  }
  if (obj->HasProperty(String::Get())) {
    JSValue* value = obj->Get(e, String::Get());
    if (!value->IsCallable() && !value->IsUndefined()) {
      *e = *Error::TypeError(u"getter not callable.");
    }
    desc->SetGet(value);
  }
  if (obj->HasProperty(String::Set())) {
    JSValue* value = obj->Get(e, String::Set());
    if (!value->IsCallable() && !value->IsUndefined()) {
      *e = *Error::TypeError(u"setter not callable.");
    }
    desc->SetSet(value);
  }
  if (desc->HasSet() || desc->HasGet()) {
    if (desc->HasValue() || desc->HasWritable()) {
      *e = *Error::TypeError(u"cannot have both get/set and value/writable");
      return nullptr;
    }
  }
  return desc;
}

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION