#ifndef ES_OBJECT_H
#define ES_OBJECT_H

#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include <es/parser/character.h>
#include <es/types/base.h>
#include <es/types/same_value.h>
#include <es/types/property_descriptor.h>
#include <es/error.h>
#include <es/utils/helper.h>
#include <es/utils/hashmap.h>

namespace es {

typedef JSValue* (*inner_func)(Error*, JSValue*, std::vector<JSValue*>);

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
    OBJ_REGEXP,
    OBJ_JSON,
    OBJ_ERROR,

    OBJ_INNER_FUNC,
    OBJ_HOST,
    OBJ_OTHER,
  };

  static JSObject* New(
    ObjType obj_type,
    std::u16string klass,
    bool extensible,
    JSValue* primitive_value,
    bool is_constructor,
    bool is_callable,
    inner_func callable,
    size_t size
  ) {
    std::cout << "JSObject::New" << std::endl;
    JSValue* jsval = JSValue::New(JS_OBJECT, kJSObjectOffset - kJSValueOffset + size);
    SET_VALUE(jsval, kObjTypeOffset, obj_type, ObjType);
    SET_VALUE(jsval, kClassOffset, String::New(klass), String*);
    SET_VALUE(jsval, kExtensibleOffset, extensible, bool);
    SET_VALUE(jsval, kPrimitiveValueOffset, primitive_value, JSValue*);
    SET_VALUE(jsval, kIsConstructorOffset, is_constructor, bool);
    SET_VALUE(jsval, kIsCallableOffset, is_callable, bool);
    // NOTE(zhuzilin) function pointer is different.
    TYPED_PTR(jsval, kCallableOffset, inner_func)[0] = callable;
    SET_VALUE(jsval, kPrototypeOffset, Null::Instance(), JSValue*);
    SET_VALUE(jsval, kNamedPropertiesOffset, HashMap<PropertyDescriptor>::New(), HashMap<PropertyDescriptor>*);
    JSObject* jsobj = new (jsval) JSObject();
    std::cout << jsobj->Class()->ToString() << " vs " << log::ToString(klass) << std::endl;
    return jsobj;
  }

  std::vector<void*> Pointers() override {
    return {
      HEAP_PTR(kClassOffset),
      HEAP_PTR(kPrimitiveValueOffset),
      HEAP_PTR(kPrototypeOffset),
      HEAP_PTR(kNamedPropertiesOffset) 
    };
  }

  ObjType obj_type() { return READ_VALUE(this, kObjTypeOffset, ObjType); }

  bool IsFunction() { return obj_type() == OBJ_FUNC; }

  // Internal Preperties Common to All Objects
  JSValue* Prototype() { return READ_VALUE(this, kPrototypeOffset, JSValue*); }
  void SetPrototype(JSValue* proto) {
    assert(proto->IsPrototype());
    SET_VALUE(this, kPrototypeOffset, proto, JSValue*);
  }
  String* Class() { return READ_VALUE(this, kClassOffset, String*); };
  bool Extensible() { return READ_VALUE(this, kExtensibleOffset, bool); };
  void SetExtensible(bool extensible) { SET_VALUE(this, kExtensibleOffset, extensible, bool); }

  virtual JSValue* Get(Error* e, String* P);
  virtual JSValue* GetOwnProperty(String* P);
  JSValue* GetProperty(String* P);
  void Put(Error* e, String* P, JSValue* V, bool throw_flag);
  bool CanPut(String* P);
  bool HasProperty(String* P);
  virtual bool Delete(Error* e, String* P, bool throw_flag);
  JSValue* DefaultValue(Error* e, std::u16string hint);
  virtual bool DefineOwnProperty(Error* e, String* P, PropertyDescriptor* desc, bool throw_flag);

  // Internal Properties Only Defined for Some Objects
  // [[PrimitiveValue]]
  JSValue* PrimitiveValue() {
    JSValue* primitive_value = READ_VALUE(this, kPrimitiveValueOffset, JSValue*);
    assert(primitive_value != nullptr);
    return primitive_value;
  };
  bool HasPrimitiveValue() {
    return obj_type() == OBJ_BOOL || obj_type() == OBJ_DATE ||
           obj_type() == OBJ_NUMBER || obj_type() == OBJ_STRING;
  }
  // [[Construct]]
  virtual JSObject* Construct(Error* e, std::vector<JSValue*> arguments) {
    assert(false);
  }
  bool IsConstructor() override { return READ_VALUE(this, kIsConstructorOffset, bool); }
  // [[Call]]
  virtual JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) {
    inner_func callable = TYPED_PTR(this, kCallableOffset, inner_func)[0];
    assert(IsCallable() && callable != nullptr);
    return callable(e, this, arguments);
  }
  bool IsCallable() override { return READ_VALUE(this, kIsCallableOffset, bool); }
  // [[HasInstance]]
  // NOTE(zhuzilin) Here we use the implementation in 15.3.5.3 [[HasInstance]] (V)
  // to make sure all callables have HasInstance.
  virtual bool HasInstance(Error* e, JSValue* V) {
    assert(IsCallable());
    if (!V->IsObject())
      return false;
    JSValue* O = Get(e, String::Prototype());
    if (!e->IsOk()) return false;
    if (!O->IsObject()) {
      *e = *Error::TypeError();
      return false;
    }
    while (!V->IsNull()) {
      if (V == O)
        return true;
      assert(V->IsObject());
      V = static_cast<JSObject*>(V)->Prototype();
      if (!e->IsOk()) return false;
    }
    return false;
  }

  void AddValueProperty(
    std::u16string name, JSValue* value, bool writable,
    bool enumerable, bool configurable
  ) {
    return AddValueProperty(String::New(name), value, writable, enumerable, configurable);
  }

  void AddValueProperty(
    String* name, JSValue* value, bool writable,
    bool enumerable, bool configurable
  ) {
    PropertyDescriptor* desc = PropertyDescriptor::New();
    desc->SetDataDescriptor(value, writable, enumerable, configurable);
    // This should just like named_properties_[name] = desc
    DefineOwnProperty(nullptr, name, desc, false);
  }

  void AddFuncProperty(
    std::u16string name, inner_func callable, bool writable,
    bool enumerable, bool configurable
  );

  // This for for-in statement.
  virtual std::vector<std::pair<String*, PropertyDescriptor*>> AllEnumerableProperties() {
    auto filter = [](PropertyDescriptor* desc) {
      return desc->HasEnumerable() && desc->Enumerable();
    };
    auto result = named_properties()->SortedKeyValPairs(filter);
    if (!Prototype()->IsNull()) {
      JSObject* proto = static_cast<JSObject*>(Prototype());
      for (auto pair : proto->AllEnumerableProperties()) {
        if (!pair.second->HasEnumerable() || !pair.second->Enumerable())
          continue;
        if (named_properties()->Get(pair.first) == nullptr) {
          result.emplace_back(pair);
        }
      }
    }
    return result;
  }

  virtual std::string ToString() override { return log::ToString(Class()->data()); }

 protected:
  static constexpr size_t kObjTypeOffset = kJSValueOffset;
  static constexpr size_t kClassOffset = kObjTypeOffset + kIntSize;
  static constexpr size_t kExtensibleOffset = kClassOffset + kPtrSize;
  static constexpr size_t kPrimitiveValueOffset = kExtensibleOffset + kBoolSize;
  static constexpr size_t kIsConstructorOffset = kPrimitiveValueOffset + kPtrSize;
  static constexpr size_t kIsCallableOffset = kIsConstructorOffset + kBoolSize;
  static constexpr size_t kCallableOffset = kIsCallableOffset + kBoolSize;
  static constexpr size_t kPrototypeOffset = kCallableOffset + kFuncPtrSize;
  static constexpr size_t kNamedPropertiesOffset = kPrototypeOffset + kPtrSize;
  static constexpr size_t kJSObjectOffset = kNamedPropertiesOffset + kPtrSize;

 private:
  HashMap<PropertyDescriptor>* named_properties() { return READ_VALUE(this, kNamedPropertiesOffset, HashMap<PropertyDescriptor>*); };
};

// 8.12.1 [[GetOwnProperty]] (P)
JSValue* JSObject::GetOwnProperty(String* P) {
  // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
  JSValue* val = named_properties()->Get(P);
  if (val == nullptr) {
    return Undefined::Instance();
  }
  // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
  // And assign the property to it. However, if we init D->value = a, and
  // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
  // remain b and that is not what we want.
  return val;
}

JSValue* JSObject::GetProperty(String* P) {
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

JSValue* JSObject::Get(Error* e, String* P) {
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

bool JSObject::CanPut(String* P) {
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
void JSObject::Put(Error* e, String* P, JSValue* V, bool throw_flag) {
  assert(V->IsLanguageType());
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
      PropertyDescriptor* value_desc = PropertyDescriptor::New();
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
  PropertyDescriptor* new_desc = PropertyDescriptor::New();
  new_desc->SetDataDescriptor(V, true, true, true);  // 6.a
  DefineOwnProperty(e, P, new_desc, throw_flag);
}

bool JSObject::HasProperty(String* P) {
  JSValue* desc = GetOwnProperty(P);
  return !desc->IsUndefined();
}

bool JSObject::Delete(Error* e, String* P, bool throw_flag) {
  JSValue* value = GetOwnProperty(P);
  if (value->IsUndefined()) {
    return true;
  }
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
  if (desc->Configurable()) {
    named_properties()->Delete(P);
    return true;
  } else {
    if (throw_flag) {
      *e = *Error::TypeError();
    }
    return false;
  }
}

// 8.12.9 [[DefineOwnProperty]] (P, Desc, Throw)
bool JSObject::DefineOwnProperty(
  Error* e, String* P, PropertyDescriptor* desc, bool throw_flag
) {
  JSValue* current = GetOwnProperty(P);
  PropertyDescriptor* current_desc;
  if (current->IsUndefined()) {
    if(!Extensible())  // 3
      goto reject;
    // 4.
    named_properties()->Set(P, desc);
    return true;
  }
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
  log::PrintSource("desc: " + desc->ToString() + ", current: " + current_desc->ToString());
  if (!current_desc->Configurable()) { // 7
    if (desc->Configurable()) {  // 7.a
      log::PrintSource("DefineOwnProperty: " + P->ToString() + " not configurable, while new value configurable");
      goto reject;
    }
    if (desc->HasEnumerable() && (desc->Enumerable() != current_desc->Enumerable())) {  // 7.b
      log::PrintSource("DefineOwnProperty: " + P->ToString() + " enumerable value differ");
      goto reject;
    }
  }
  // 8.
  if (!desc->IsGenericDescriptor()) {
    if (current_desc->IsDataDescriptor() != desc->IsDataDescriptor()) {  // 9.
      // 9.a
      if (!current_desc->Configurable()) goto reject;
      // 9.b.i & 9.c.i
      PropertyDescriptor* old_property = named_properties()->Get(P);
      PropertyDescriptor* new_property = PropertyDescriptor::New();
      new_property->SetConfigurable(old_property->Configurable());
      new_property->SetEnumerable(old_property->Enumerable());
      new_property->SetBitMask(old_property->bitmask());
      named_properties()->Set(P, new_property);
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
  log::PrintSource("DefineOwnProperty: " + P->ToString() + " is set to " + desc->Value()->ToString());
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

bool JSValue::IsNumberObject() {
  return IsObject() ? (static_cast<JSObject*>(this)->obj_type() == JSObject::OBJ_NUMBER) : false;
}

bool JSValue::IsArrayObject() {
  return IsObject() ? (static_cast<JSObject*>(this)->obj_type() == JSObject::OBJ_ARRAY) : false;
}

bool JSValue::IsRegExpObject() {
  return IsObject() ? (static_cast<JSObject*>(this)->obj_type() == JSObject::OBJ_REGEXP) : false;
}

bool JSValue::IsErrorObject() {
  return IsObject() ? (static_cast<JSObject*>(this)->obj_type() == JSObject::OBJ_ERROR) : false;
}

}  // namespace

#endif  // ES_OBJECT_H