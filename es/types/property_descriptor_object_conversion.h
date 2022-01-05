#ifndef ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION
#define ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION

#include <es/types/property_descriptor.h>
#include <es/types/object.h>
#include <es/error.h>

namespace es {

JSValue* FromPropertyDescriptor(JSValue* value, Error* e) {
  if (value->IsUndefined()) {
    return JSUndefined::Instance();
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  JSObject* obj = new JSObject(JSObject::OBJ_OBJECT);
  if (desc->IsDataDescriptor()) {
    PropertyDescriptor* value_desc = new PropertyDescriptor();
    value_desc->SetDataDescriptor(desc->Value(), true, true, true);
    obj->DefineOwnProperty(u"value", value_desc, false, e);

    PropertyDescriptor* writable_desc = new PropertyDescriptor();
    writable_desc->SetDataDescriptor(JSBool::Wrap(desc->Writable()), true, true, true);
    obj->DefineOwnProperty(u"writable", writable_desc, false, e);
  } else {
    assert(desc->IsAccessorDescriptor());
    PropertyDescriptor* get_desc = new PropertyDescriptor();
    get_desc->SetDataDescriptor(desc->Get(), true, true, true);
    obj->DefineOwnProperty(u"get", get_desc, false, e);

    PropertyDescriptor* set_desc = new PropertyDescriptor();
    set_desc->SetDataDescriptor(desc->Set(), true, true, true);
    obj->DefineOwnProperty(u"set", set_desc, false, e);
  }

  PropertyDescriptor* enumerable_desc = new PropertyDescriptor();
  enumerable_desc->SetDataDescriptor(JSBool::Wrap(desc->Enumerable()), true, true, true);
  obj->DefineOwnProperty(u"get", enumerable_desc, false, e);

  PropertyDescriptor* configurable_desc = new PropertyDescriptor();
  configurable_desc->SetDataDescriptor(JSBool::Wrap(desc->Configurable()), true, true, true);
  obj->DefineOwnProperty(u"set", configurable_desc, false, e);

  return obj;
}

JSValue* ToPropertyDescriptor(JSValue* obj, Error* e) {
  if (!obj->IsObject()) {
    e = Error::TypeError();
    return;
  }
  PropertyDescriptor* desc = new PropertyDescriptor();
  // TODO(zhuzilin)
  return desc;
}

}  // namespace es

#endif  // ES_TYPES_PROPERTY_DESCRIPTOR_OBJECT_CONVERSION