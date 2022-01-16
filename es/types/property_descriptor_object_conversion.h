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

PropertyDescriptor* ToPropertyDescriptor(Error* e, JSValue* val) {
  if (!val->IsObject()) {
    *e = *Error::TypeError();
    return nullptr;
  }
  JSObject* obj = static_cast<JSObject*>(val);
  PropertyDescriptor* desc = new PropertyDescriptor();
  if (obj->HasProperty(u"enumerable")) {
    JSValue* value = obj->Get(e, u"enumerable");
    desc->SetEnumerable(ToBoolean(value));
  }
  if (obj->HasProperty(u"configurable")) {
    JSValue* value = obj->Get(e, u"configurable");
    desc->SetConfigurable(ToBoolean(value));
  }
  if (obj->HasProperty(u"value")) {
    JSValue* value = obj->Get(e, u"value");
    desc->SetValue(value);
  }
  if (obj->HasProperty(u"writable")) {
    JSValue* value = obj->Get(e, u"writable");
    desc->SetWritable(ToBoolean(value));
  }
  if (obj->HasProperty(u"get")) {
    JSValue* value = obj->Get(e, u"get");
    if (!value->IsCallable() && !value->IsUndefined()) {
      *e = *Error::TypeError(u"getter not callable.");
    }
    desc->SetGet(value);
  }
  if (obj->HasProperty(u"set")) {
    JSValue* value = obj->Get(e, u"set");
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