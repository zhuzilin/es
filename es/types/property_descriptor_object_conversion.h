#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION
#define ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION

#include <es/types/property_descriptor.h>
#include <es/types/builtin/object_object.h>
#include <es/error.h>

namespace es {

JSValue* FromPropertyDescriptor(Error* e, JSValue* value) {
  if (value->IsUndefined()) {
    return Undefined::Instance();
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  JSObject* obj = new Object();
  if (desc->IsDataDescriptor()) {
    PropertyDescriptor* value_desc = new PropertyDescriptor();
    value_desc->SetDataDescriptor(desc->Value(), true, true, true);
    obj->DefineOwnProperty(e, u"value", value_desc, false);
    if (!e->IsOk()) return nullptr;

    PropertyDescriptor* writable_desc = new PropertyDescriptor();
    writable_desc->SetDataDescriptor(Bool::Wrap(desc->Writable()), true, true, true);
    obj->DefineOwnProperty(e, u"writable", writable_desc, false);
    if (!e->IsOk()) return nullptr;
  } else {
    assert(desc->IsAccessorDescriptor());
    PropertyDescriptor* get_desc = new PropertyDescriptor();
    get_desc->SetDataDescriptor(desc->Get(), true, true, true);
    obj->DefineOwnProperty(e, u"get", get_desc, false);
    if (!e->IsOk()) return nullptr;

    PropertyDescriptor* set_desc = new PropertyDescriptor();
    set_desc->SetDataDescriptor(desc->Set(), true, true, true);
    obj->DefineOwnProperty(e, u"set", set_desc, false);
    if (!e->IsOk()) return nullptr;
  }

  PropertyDescriptor* enumerable_desc = new PropertyDescriptor();
  enumerable_desc->SetDataDescriptor(Bool::Wrap(desc->Enumerable()), true, true, true);
  obj->DefineOwnProperty(e, u"get", enumerable_desc, false);
  if (!e->IsOk()) return nullptr;

  PropertyDescriptor* configurable_desc = new PropertyDescriptor();
  configurable_desc->SetDataDescriptor(Bool::Wrap(desc->Configurable()), true, true, true);
  obj->DefineOwnProperty(e, u"set", configurable_desc, false);
  if (!e->IsOk()) return nullptr;

  return obj;
}

JSValue* ToPropertyDescriptor(Error* e, JSValue* obj) {
  if (!obj->IsObject()) {
    *e = *Error::TypeError();
    return;
  }
  PropertyDescriptor* desc = new PropertyDescriptor();
  // TODO(zhuzilin)
  return desc;
}

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION