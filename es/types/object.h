#ifndef ES_OBJECT_H
#define ES_OBJECT_H

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include <es/types/base.h>
#include <es/types/same_value.h>
#include <es/types/property_descriptor.h>
#include <es/error.h>

namespace es {


class JSObject : public JSValue {
 public:
  enum ObjType {
    OBJ_GLOBAL,
    OBJ_OBJECT,
    OBJ_FUNC,
    OBJ_ARRAY,
    OBJ_STRING,
    OBJ_BOOL,
    OBJ_NUMBER,
    OBJ_MATH,
    OBJ_DATE,
    OBJ_REGEX,
    OBJ_JSON,
    OBJ_ERROR,

    OBJ_OTHER,
  };

  JSObject(
    ObjType obj_type,
    JSValue* prototype,
    std::u16string_view klass,
    bool extensible,
    JSValue* primitive_value,
    bool is_constructor,
    bool is_callable
  ) : JSValue(JS_OBJECT), obj_type_(obj_type), class_(klass), extensible_(extensible),
      primitive_value_(primitive_value), is_constructor_(is_constructor),
      is_callable_(is_callable) {
    assert(prototype->type() == JS_OBJECT || prototype->type() == JS_NULL);
    prototype_ = prototype;
  }

  ObjType obj_type() { return obj_type_; }

  bool IsFunction() { return obj_type_ == OBJ_FUNC; }

  // Internal Preperties Common to All Objects
  JSValue* Prototype() { return prototype_; }
  std::u16string_view Class() { return class_; };
  bool Extensible() { return extensible_; };

  JSValue* Get(std::u16string_view p);
  JSValue* GetOwnProperty(std::u16string_view p);
  JSValue* GetProperty(std::u16string_view p);
  void Put(std::u16string_view p, JSValue* v, bool throw_flag, Error *e);
  bool CanPut(std::u16string_view p);
  bool HasProperty(std::u16string_view p);
  bool Delete(std::u16string_view p, bool throw_flag, Error *e);
  JSValue* DefaultValue(std::u16string_view hint, Error *e);
  bool DefineOwnProperty(std::u16string_view p, PropertyDescriptor* desc, bool throw_flag, Error *e);

  // Internal Properties Only Defined for Some Objects
  // [[PrimitiveValue]]
  JSValue* PrimitiveValue() {
    assert(primitive_value_ != nullptr);
    return primitive_value_;
  };
  bool HasPrimitiveValue() {
    return obj_type_ == OBJ_BOOL || obj_type_ == OBJ_DATE ||
           obj_type_ == OBJ_NUMBER || obj_type_ == OBJ_STRING;
  }
  // [[Construct]]
  virtual JSObject* Construct(std::vector<JSValue*> arguments) {
    assert(false);
  }
  bool IsConstructor() { return is_constructor_; }
  // [[Call]]
  virtual JSValue* Call(JSValue* argument, std::vector<JSValue*> arguments = {}) {
    assert(false);
  }
  bool IsCallable() override { return is_callable_; }
  // [[HasInstance]]
  virtual bool HasInstance(JSValue* value) {
    assert(false);
  }

 protected:
  std::unordered_map<std::u16string_view, PropertyDescriptor*> named_properties_;

 private:  
  ObjType obj_type_;

  JSValue* prototype_;
  std::u16string_view class_;
  bool extensible_;

  JSValue* primitive_value_;

  bool is_constructor_;
  bool is_callable_;
};

// 8.12.1 [[GetOwnProperty]] (P)
JSValue* JSObject::GetOwnProperty(std::u16string_view p) {
  // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
  auto iter = named_properties_.find(p);
  if (iter == named_properties_.end()) {
    return Undefined::Instance();
  }
  // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
  // And assign the property to it. However, if we init D->value = a, and
  // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
  // remain b and that is not what we want.
  return iter->second;
}

JSValue* JSObject::GetProperty(std::u16string_view p) {
  JSValue* own_property = GetOwnProperty(p);
  if (!own_property->IsUndefined()) {
    return own_property;
  }
  JSValue* proto = Prototype();
  if (proto->IsNull()) {
    return Undefined::Instance();
  }
  assert(proto->IsObject());
  JSObject* proto_obj = static_cast<JSObject*>(proto);
  return proto_obj->GetProperty(p);
}

JSValue* JSObject::Get(std::u16string_view p) {
  JSValue* value = GetProperty(p);
  if (value->IsUndefined()) {
    return Undefined::Instance();
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  if (desc->IsDataDescriptor()) {
    return desc->Value();
  } else {
    assert(desc->IsAccessorDescriptor());
    JSValue* getter = desc->Get();
    if (getter->IsUndefined()) {
      return Undefined::Instance();
    }
    JSObject* getter_obj = static_cast<JSObject*>(getter);
    return getter_obj->Call(this);
  }
}

bool JSObject::CanPut(std::u16string_view p) {
  log::PrintSource("CanPut ", p);
  JSValue* value = GetOwnProperty(p);
  if (!value->IsUndefined()) {
    PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
    if (desc->IsAccessorDescriptor()) {
      return !desc->Set()->IsUndefined();
    } else {
      return desc->Writable();
    }
  }

  JSValue* proto = Prototype();
  if (proto->IsNull()) {
    return Extensible();
  }
  JSObject* proto_obj = static_cast<JSObject*>(proto);
  JSValue* inherit = proto_obj->GetProperty(p);
  if (inherit->IsUndefined()) {
    return Extensible();
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(inherit);
  if (desc->IsAccessorDescriptor()) {
    return !desc->Set()->IsUndefined();
  } else {
    return Extensible() ? desc->Writable() : false;
  }
}

void JSObject::Put(std::u16string_view p, JSValue* v, bool throw_flag, Error *e) {
  log::PrintSource("Put ", p);
  if (!CanPut(p)) {
    if (throw_flag) {
      e = Error::TypeError();
    }
    return;
  }
  JSValue* value = GetOwnProperty(p);
  if (!value->IsUndefined()) {
    PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(value);
    if (own_desc->IsDataDescriptor()) {
      PropertyDescriptor* value_desc = new PropertyDescriptor();
      value_desc->SetValue(v);
      DefineOwnProperty(p, value_desc, throw_flag, e);
      return;
    }
  }
  value = GetProperty(p);
  if (!value->IsUndefined()) {
    PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
    if (desc->IsAccessorDescriptor()) {
      JSValue* setter = desc->Set();
      assert(!setter->IsUndefined());
      JSObject* setter_obj = static_cast<JSObject*>(setter);
      setter_obj->Call(this, {v});
      return;
    }
  }
  PropertyDescriptor* new_desc = new PropertyDescriptor();
  new_desc->SetDataDescriptor(v, true, true, true);
  DefineOwnProperty(p, new_desc, throw_flag, e);
}

bool JSObject::HasProperty(std::u16string_view p) {
  JSValue* desc = GetOwnProperty(p);
  return !desc->IsUndefined();
}

bool JSObject::Delete(std::u16string_view p, bool throw_flag, Error *e) {
  JSValue* value = GetOwnProperty(p);
  if (value->IsUndefined()) {
    return true;
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  if (desc->Configurable()) {
    named_properties_.erase(p);
    return true;
  } else {
    if (throw_flag) {
      e = Error::TypeError();
    }
    return false;
  }
}

JSValue* JSObject::DefaultValue(std::u16string_view hint, Error *e) {
  std::u16string_view first, second;
  if (hint == u"String" || hint == u"" && obj_type() == OBJ_DATE) {
    first = u"String";
    second = u"Number";
  } else if (hint == u"Number" || hint == u"" && obj_type() != OBJ_DATE) {
    first = u"Number";
    second = u"String";
  } else {
    assert(false);
  }

  JSValue* to_string = Get(first);
  if (to_string->IsCallable()) {
    JSObject* to_string_obj = static_cast<JSObject*>(to_string);
    JSValue* str = to_string_obj->Call(this);
    if (str->IsPrimitive()) {
      return str;
    }
  }
  JSValue* value_of = Get(second);
  if (value_of->IsCallable()) {
    JSObject* value_of_obj = static_cast<JSObject*>(value_of);
    JSValue* val = value_of_obj->Call(this);
    if (val->IsPrimitive()) {
      return val;
    }
  }
  e = Error::TypeError();
  return nullptr;
}

bool JSObject::DefineOwnProperty(std::u16string_view p, PropertyDescriptor* desc, bool throw_flag, Error *e) {
  log::PrintSource("DefineOwnProperty: ", p);
  JSValue* current = GetOwnProperty(p);
  if (current->IsUndefined() && !extensible_) {
    goto reject;
  }
  if (current->IsUndefined() && extensible_) {  // 4.
    log::PrintSource("DefineOwnProperty: ", p, " undefined and extensible_");
    named_properties_[p] = desc;
    return true;
  }
  if (desc->bitmask() == 0) {  // 5
    return true;
  }
  if (!current->IsUndefined()) {
    log::PrintSource("DefineOwnProperty: ", p, " defined");
    PropertyDescriptor* current_desc = static_cast<PropertyDescriptor*>(current);
    if ((desc->bitmask() & current_desc->bitmask()) == desc->bitmask()) {
      bool same = true;
      if (desc->HasValue())
        same = same && SameValue(desc->Value(), current_desc->Value());
      if (desc->HasWritable())
        same = same && (desc->Writable() == current_desc->Writable());
      if (desc->HasGet())
        same = same && SameValue(desc->Get(), current_desc->Get());
      if (desc->HasSet())
        same = same && SameValue(desc->Set(), current_desc->Set());
      if (desc->HasConfigurable())
        same = same && (desc->Configurable() == current_desc->Configurable());
      if (desc->HasEnumerable())
        same = same && (desc->Enumerable() == current_desc->Enumerable());
      if (same) return true;  // 6
    }
    log::PrintSource("DefineOwnProperty: ", p, " not same");
    if (!current_desc->Configurable()) { // 7
      log::PrintSource("DefineOwnProperty: ", p, " not configurable");
      if (desc->Configurable()) goto reject;  // 7.1
      if (desc->HasEnumerable() && (desc->Enumerable() != current_desc->Enumerable())) goto reject;  // 7.b
    }
    // 8.
    if (!desc->IsGenericDescriptor()) {
      if (current_desc->IsDataDescriptor() != desc->IsDataDescriptor()) {  // 9.
        // 9.a
        if (!current_desc->Configurable()) goto reject;
        // 9.b.i & 9.c.i
        PropertyDescriptor* old_property = named_properties_[p];
        PropertyDescriptor* new_property = new PropertyDescriptor();
        new_property->SetConfigurable(old_property->Configurable());
        new_property->SetEnumerable(old_property->Enumerable());
        new_property->SetBitMask(old_property->bitmask());
        named_properties_[p] = new_property;
        delete old_property;
      } else if (current_desc->IsDataDescriptor() && desc->IsDataDescriptor()) {  // 10.
        if (!current_desc->Configurable()) {  // 10.a
          if (!current_desc->Writable()) {
            if (desc->Writable()) goto reject;  // 10.a.i
            // 10.a.ii.1
            if (desc->HasValue() && !SameValue(desc->Value(), current_desc->Value())) goto reject;
          }
        } else {  // 10.b
          assert(current_desc->Configurable());
        }
      } else {  // 11.
        assert(current_desc->IsAccessorDescriptor() && desc->IsAccessorDescriptor());
        if (!current_desc->Configurable()) {  // 11.a
          if (!SameValue(desc->Set(), current_desc->Set()) ||  // 11.a.i
              !SameValue(desc->Get(), current_desc->Get()))    // 11.a.ii
            goto reject;
        }
      }
    }
    log::PrintSource("DefineOwnProperty: ", p, " set");
    // 12.
    current_desc->Set(desc);
  }
  // 13.
  return true;
reject:
  if (throw_flag) {
    e = Error::TypeError();
  }
  return false;
}

}  // namespace

#endif  // ES_OBJECT_H