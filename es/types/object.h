#ifndef ES_OBJECT_H
#define ES_OBJECT_H

#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include <es/types/base.h>
#include <es/types/same_value.h>
#include <es/types/property_descriptor.h>
#include <es/error.h>

namespace es {

typedef std::function<JSValue* (Error*, std::vector<JSValue*>)> inner_func;

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
    std::u16string klass,
    bool extensible,
    JSValue* primitive_value,
    bool is_constructor,
    bool is_callable,
    inner_func callable = nullptr
  ) : JSValue(JS_OBJECT), obj_type_(obj_type),
      prototype_(Null::Instance()), class_(klass), extensible_(extensible),
      primitive_value_(primitive_value), is_constructor_(is_constructor),
      is_callable_(is_callable), callable_(callable) {}

  ObjType obj_type() { return obj_type_; }
  std::unordered_map<std::u16string, PropertyDescriptor*> named_properties() { return named_properties_; }

  bool IsFunction() { return obj_type_ == OBJ_FUNC; }

  // Internal Preperties Common to All Objects
  JSValue* Prototype() { return prototype_; }
  void SetPrototype(JSValue* proto) {
    assert(proto->type() == JS_NULL || proto->type() == JS_OBJECT);
    prototype_ = proto;
  }
  std::u16string Class() { return class_; };
  bool Extensible() { return extensible_; };

  virtual JSValue* Get(Error* e, std::u16string P);
  JSValue* GetOwnProperty(std::u16string P);
  JSValue* GetProperty(std::u16string P);
  void Put(Error* e, std::u16string P, JSValue* V, bool throw_flag);
  bool CanPut(std::u16string P);
  bool HasProperty(std::u16string P);
  bool Delete(Error* e, std::u16string P, bool throw_flag);
  JSValue* DefaultValue(Error* e, std::u16string hint);
  bool DefineOwnProperty(Error* e, std::u16string P, PropertyDescriptor* desc, bool throw_flag);

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
  virtual JSObject* Construct(Error* e, std::vector<JSValue*> arguments) {
    assert(false);
  }
  bool IsConstructor() override { return is_constructor_; }
  // [[Call]]
  virtual JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) {
    assert(is_callable_ && callable_ != nullptr);
    // TODO(zhuzilin) check if any callable need this_arg.
    return callable_(e, arguments);
  }
  bool IsCallable() override { return is_callable_; }
  // [[HasInstance]]
  virtual bool HasInstance(Error* e, JSValue* value) {
    assert(false);
  }

  void AddValueProperty(
    std::u16string name, JSValue* value, bool writable,
    bool enumerable, bool configurable
  ) {
    PropertyDescriptor* desc = new PropertyDescriptor();
    desc->SetDataDescriptor(value, writable, enumerable, configurable);
    // This should just like named_properties_[name] = desc
    DefineOwnProperty(nullptr, name, desc, false);
  }

  void AddFuncProperty(
    std::u16string name, inner_func callable, bool writable,
    bool enumerable, bool configurable
  ) {
    JSObject* value = new JSObject(
      OBJ_OTHER, u"InternalFunc", false, nullptr, false, true, callable);
    AddValueProperty(name, value, writable, enumerable, configurable);
  }

  virtual std::string ToString() override { return log::ToString(class_); }

 private:  
  ObjType obj_type_;
  std::unordered_map<std::u16string, PropertyDescriptor*> named_properties_;

  JSValue* prototype_;
  std::u16string class_;
  bool extensible_;

  JSValue* primitive_value_;

  bool is_constructor_;
  bool is_callable_;
  inner_func callable_;
};

// 8.12.1 [[GetOwnProperty]] (P)
JSValue* JSObject::GetOwnProperty(std::u16string P) {
  // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
  auto iter = named_properties_.find(P);
  if (iter == named_properties_.end()) {
    return Undefined::Instance();
  }
  // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
  // And assign the property to it. However, if we init D->value = a, and
  // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
  // remain b and that is not what we want.
  return iter->second;
}

JSValue* JSObject::GetProperty(std::u16string P) {
  JSValue* own_property = GetOwnProperty(P);
  if (!own_property->IsUndefined()) {
    return own_property;
  }
  JSValue* proto = Prototype();
  if (proto->IsNull()) {
    return Undefined::Instance();
  }
  assert(proto->IsObject());
  JSObject* proto_obj = static_cast<JSObject*>(proto);
  return proto_obj->GetProperty(P);
}

JSValue* JSObject::Get(Error* e, std::u16string P) {
  JSValue* value = GetProperty(P);
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
    return getter_obj->Call(e, this);
  }
}

bool JSObject::CanPut(std::u16string P) {
  JSValue* value = GetOwnProperty(P);
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
  JSValue* inherit = proto_obj->GetProperty(P);
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

// 8.12.5 [[Put]] ( P, V, Throw )
void JSObject::Put(Error* e, std::u16string P, JSValue* V, bool throw_flag) {
  log::PrintSource("Put ", P, " " + V->ToString());
  if (!CanPut(P)) {  // 1
    if (throw_flag) {  // 1.a
      *e = *Error::TypeError();
    }
    return;  // 1.b
  }
  JSValue* value = GetOwnProperty(P);
  if (!value->IsUndefined()) {
    PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(value);  // 2
    if (own_desc->IsDataDescriptor()) {  // 3
      PropertyDescriptor* value_desc = new PropertyDescriptor();
      value_desc->SetValue(V);
      log::PrintSource("Overwrite the old desc with " + value_desc->ToString());
      DefineOwnProperty(e, P, value_desc, throw_flag);
      return;
    }
  }
  value = GetProperty(P);
  if (!value->IsUndefined()) {
    PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
    if (desc->IsAccessorDescriptor()) {
      log::PrintSource("Use parent prototype's setter");
      JSValue* setter = desc->Set();
      assert(!setter->IsUndefined());
      JSObject* setter_obj = static_cast<JSObject*>(setter);
      setter_obj->Call(e, this, {V});
      return;
    }
  }
  PropertyDescriptor* new_desc = new PropertyDescriptor();
  new_desc->SetDataDescriptor(V, true, true, true);  // 6.a
  DefineOwnProperty(e, P, new_desc, throw_flag);
}

bool JSObject::HasProperty(std::u16string P) {
  JSValue* desc = GetOwnProperty(P);
  return !desc->IsUndefined();
}

bool JSObject::Delete(Error* e, std::u16string P, bool throw_flag) {
  JSValue* value = GetOwnProperty(P);
  if (value->IsUndefined()) {
    return true;
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  if (desc->Configurable()) {
    named_properties_.erase(P);
    return true;
  } else {
    if (throw_flag) {
      *e = *Error::TypeError();
    }
    return false;
  }
}

JSValue* JSObject::DefaultValue(Error* e, std::u16string hint) {
  std::u16string first, second;
  if (hint == u"String" || hint == u"" && obj_type() == OBJ_DATE) {
    first = u"String";
    second = u"Number";
  } else if (hint == u"Number" || hint == u"" && obj_type() != OBJ_DATE) {
    first = u"Number";
    second = u"String";
  } else {
    assert(false);
  }

  JSValue* to_string = Get(e, first);
  if (!e->IsOk()) return nullptr;
  if (to_string->IsCallable()) {
    JSObject* to_string_obj = static_cast<JSObject*>(to_string);
    JSValue* str = to_string_obj->Call(e, this);
    if (!e->IsOk()) return nullptr;
    if (str->IsPrimitive()) {
      return str;
    }
  }
  JSValue* value_of = Get(e, second);
  if (!e->IsOk()) return nullptr;
  if (value_of->IsCallable()) {
    JSObject* value_of_obj = static_cast<JSObject*>(value_of);
    JSValue* val = value_of_obj->Call(e, this);
    if (!e->IsOk()) return nullptr;
    if (val->IsPrimitive()) {
      return val;
    }
  }
  *e = *Error::TypeError();
  return nullptr;
}

// 8.12.9 [[DefineOwnProperty]] (P, Desc, Throw)
bool JSObject::DefineOwnProperty(
  Error* e, std::u16string P, PropertyDescriptor* desc, bool throw_flag
) {
  log::PrintSource("enter DefineOwnProperty");
  JSValue* current = GetOwnProperty(P);
  PropertyDescriptor* current_desc;
  if (current->IsUndefined()) {
    if(!extensible_)  // 3
      goto reject;
     // 4.
    log::PrintSource("DefineOwnProperty: ", P, " has no desc and extensible, set to " + desc->ToString());
    named_properties_[P] = desc;
    return true;
  }
  log::PrintSource("DefineOwnProperty: ", P, " defined");
  if (desc->bitmask() == 0) {  // 5
    return true;
  }
  current_desc = static_cast<PropertyDescriptor*>(current);
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
  std::cout << "desc: " << desc->ToString() << ", current: " << current_desc->ToString() << std::endl;
  if (!current_desc->Configurable()) { // 7
    if (desc->Configurable()) {  // 7.a
      log::PrintSource("DefineOwnProperty: ", P, " not configurable, while new value configurable");
      goto reject;
    }
    if (desc->HasEnumerable() && (desc->Enumerable() != current_desc->Enumerable())) {  // 7.b
      log::PrintSource("DefineOwnProperty: ", P, " enumerable value differ");
      goto reject;
    }
  }
  // 8.
  if (!desc->IsGenericDescriptor()) {
    if (current_desc->IsDataDescriptor() != desc->IsDataDescriptor()) {  // 9.
      // 9.a
      if (!current_desc->Configurable()) goto reject;
      // 9.b.i & 9.c.i
      PropertyDescriptor* old_property = named_properties_[P];
      PropertyDescriptor* new_property = new PropertyDescriptor();
      new_property->SetConfigurable(old_property->Configurable());
      new_property->SetEnumerable(old_property->Enumerable());
      new_property->SetBitMask(old_property->bitmask());
      named_properties_[P] = new_property;
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
  log::PrintSource("DefineOwnProperty: ", P, " is set" +
                    (desc->HasValue() ? " to " + desc->Value()->ToString() : ""));
  // 12.
  current_desc->Set(desc);
  // 13.
  return true;
reject:
  log::PrintSource("DefineOwnProperty reject");
  if (throw_flag) {
    *e = *Error::TypeError();
  }
  return false;
}

}  // namespace

#endif  // ES_OBJECT_H