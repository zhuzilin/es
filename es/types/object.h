#ifndef ES_OBJECT_H
#define ES_OBJECT_H

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include <es/types/base.h>
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

  struct NamedProperty {
    enum Type {
      DATA,
      ACCESSOR,
    };

    NamedProperty(Type t) : type(t) {}

    Type type;
  };

  struct NamedDataProperty : public NamedProperty {
    NamedDataProperty(
      JSValue* v = Undefined::Instance(),
      bool w = false,
      bool e = false,
      bool c = false
    ) : NamedProperty(DATA), value(v), writable(w),
        enumerable(e), configurable(c) {}

    JSValue* value;
    bool writable;
    bool enumerable;
    bool configurable;
  };

  struct NamedAccessorProperty : public NamedProperty {
    NamedAccessorProperty(
      JSValue* g = Undefined::Instance(),
      JSValue* s = Undefined::Instance(),
      bool e = false,
      bool c = false
    ) : NamedProperty(ACCESSOR), enumerable(e), configurable(c) {
      assert(g->IsUndefined() || g->IsObject());
      assert(s->IsUndefined() || s->IsObject());
      // TODO(zhuzilin) Check getter and setter are functions.
      getter = g;
      setter = s;
    }

    JSValue* getter;
    JSValue* setter;
    bool enumerable;
    bool configurable;
  };

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
  std::unordered_map<std::u16string_view, NamedProperty*> named_properties_;

 private:  
  ObjType obj_type_;

  JSValue* prototype_;
  std::u16string_view class_;
  bool extensible_;

  JSValue* primitive_value_;

  bool is_constructor_;
  bool is_callable_;
};

JSValue* JSObject::GetOwnProperty(std::u16string_view p) {
  // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
  auto iter = named_properties_.find(p);
  if (iter == named_properties_.end()) {
    return Undefined::Instance();
  }
  PropertyDescriptor* D = new PropertyDescriptor();
  NamedProperty* X = iter->second;
  if (X->type == NamedProperty::DATA) {
    NamedDataProperty* dp = static_cast<NamedDataProperty*>(X);
    D->SetDataDescriptor(dp->value, dp->writable, dp->enumerable, dp->configurable);
  } else {
    assert(X->type == NamedProperty::ACCESSOR);
    NamedAccessorProperty* ap = static_cast<NamedAccessorProperty*>(X);
    D->SetDataDescriptor(ap->getter, ap->setter, ap->enumerable, ap->configurable);
  }
  return D;
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

bool SameValue(JSValue* x, JSValue* y);

bool JSObject::DefineOwnProperty(std::u16string_view p, PropertyDescriptor* desc, bool throw_flag, Error *e) {
  JSValue* current = GetOwnProperty(p);
  if (current->IsUndefined() && !extensible_) {
    goto reject;
  }
  if (current->IsUndefined() && extensible_) {
    if (desc->IsGenericDescriptor() || desc->IsDataDescriptor()) {
      NamedDataProperty* data_property = new NamedDataProperty();
      if (desc->HasValue()) data_property->value = desc->Value();
      if (desc->HasWritable()) data_property->writable = desc->Writable();
      if (desc->HasEnumerable()) data_property->enumerable = desc->Enumerable();
      if (desc->HasConfigurable()) data_property->configurable = desc->Configurable();
      named_properties_[p] = data_property;
    } else {
      assert(desc->IsAccessorDescriptor());
      NamedAccessorProperty* accessor_property = new NamedAccessorProperty();
      if (desc->HasGet()) accessor_property->getter = desc->Get();
      if (desc->HasSet()) accessor_property->setter = desc->Set();
      if (desc->HasEnumerable()) accessor_property->enumerable = desc->Enumerable();
      if (desc->HasConfigurable()) accessor_property->configurable = desc->Configurable();
      named_properties_[p] = accessor_property;
    }
    return true;
  }
  if (desc->bitmask() == 0) {
    return true;
  }
  if (!current->IsUndefined()) {
    PropertyDescriptor* current_desc = static_cast<PropertyDescriptor*>(current);
    if ((desc->bitmask() & current_desc->bitmask()) == desc->bitmask()) {
      bool same = true;
      // TODO(zhuzilin)

      if (same) return true;
    }

    if (!current_desc->Configurable()) {
      if (desc->Configurable()) goto reject;
      if (desc->HasEnumerable() && (desc->Enumerable() != current_desc->Enumerable())) goto reject;
    }
    // 8.
    if (!desc->IsGenericDescriptor()) {
      if (current_desc->IsDataDescriptor() != desc->IsDataDescriptor()) {  // 9.
        // 9.a
        if (!current_desc->Configurable()) goto reject;
        if (current_desc->IsDataDescriptor()) {  // 9.b.i
          NamedDataProperty* data_property = static_cast<NamedDataProperty*>(named_properties_[p]);
          NamedAccessorProperty* accessor_property = new NamedAccessorProperty();
          accessor_property->configurable = data_property->configurable;
          accessor_property->enumerable = data_property->enumerable;
          named_properties_[p] = accessor_property;
        } else {  // 9.c.i
          NamedAccessorProperty* accessor_property = static_cast<NamedAccessorProperty*>(named_properties_[p]);
          NamedDataProperty* data_property = new NamedDataProperty();
          data_property->configurable = accessor_property->configurable;
          data_property->enumerable = accessor_property->enumerable;
          named_properties_[p] = data_property;
        }
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
    // 12.
    current_desc->Set(desc);
  }
  
  return true;
reject:
  if (throw_flag) {
    e = Error::TypeError();
  }
  return false;
}

}  // namespace

#endif  // ES_OBJECT_H