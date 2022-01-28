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

typedef Handle<JSValue> (*inner_func)(Error*, Handle<JSValue>, std::vector<Handle<JSValue>>);

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

  static Handle<JSObject> New(
    ObjType obj_type,
    std::u16string klass,
    bool extensible,
    Handle<JSValue> primitive_value,
    bool is_constructor,
    bool is_callable,
    inner_func callable,
    size_t size
  ) {
    std::cout << "JSObject::New" << std::endl;
    Handle<JSValue> jsval = JSValue::New(JS_OBJECT, kJSObjectOffset - kJSValueOffset + size);
    SET_VALUE(jsval.val(), kObjTypeOffset, obj_type, ObjType);
    SET_HANDLE_VALUE(jsval.val(), kClassOffset, String::New(klass), String);
    SET_VALUE(jsval.val(), kExtensibleOffset, extensible, bool);
    SET_HANDLE_VALUE(jsval.val(), kPrimitiveValueOffset, primitive_value, JSValue);
    SET_VALUE(jsval.val(), kIsConstructorOffset, is_constructor, bool);
    SET_VALUE(jsval.val(), kIsCallableOffset, is_callable, bool);
    // NOTE(zhuzilin) function pointer is different.
    TYPED_PTR(jsval.val(), kCallableOffset, inner_func)[0] = callable;
    SET_HANDLE_VALUE(jsval.val(), kPrototypeOffset, Null::Instance(), JSValue);
    SET_HANDLE_VALUE(jsval.val(), kNamedPropertiesOffset, HashMap<PropertyDescriptor>::New(), HashMap<PropertyDescriptor>);
    new (jsval.val()) JSObject();
    std::cout << "return JSObject::New" << std::endl;
    return Handle<JSObject>(jsval);
  }

  std::vector<HeapObject**> Pointers() override {
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
  Handle<JSValue> Prototype() { return READ_HANDLE_VALUE(this, kPrototypeOffset, JSValue); }
  void SetPrototype(Handle<JSValue> proto) {
    assert(proto.val()->IsPrototype());
    SET_HANDLE_VALUE(this, kPrototypeOffset, proto, JSValue);
  }
  Handle<String> Class() { return READ_HANDLE_VALUE(this, kClassOffset, String); };
  bool Extensible() { return READ_VALUE(this, kExtensibleOffset, bool); };
  void SetExtensible(bool extensible) { SET_VALUE(this, kExtensibleOffset, extensible, bool); }

  virtual Handle<JSValue> Get(Error* e, Handle<String> P);
  virtual Handle<JSValue> GetOwnProperty(Handle<String> P);
  Handle<JSValue> GetProperty(Handle<String> P);
  void Put(Error* e, Handle<String> P, Handle<JSValue> V, bool throw_flag);
  bool CanPut(Handle<String> P);
  bool HasProperty(Handle<String> P);
  virtual bool Delete(Error* e, Handle<String> P, bool throw_flag);
  Handle<JSValue> DefaultValue(Error* e, std::u16string hint);
  virtual bool DefineOwnProperty(Error* e, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag);

  // Internal Properties Only Defined for Some Objects
  // [[PrimitiveValue]]
  Handle<JSValue> PrimitiveValue() {
    Handle<JSValue> primitive_value = READ_HANDLE_VALUE(this, kPrimitiveValueOffset, JSValue);
    assert(!primitive_value.IsNullptr());
    return primitive_value;
  };
  bool HasPrimitiveValue() {
    return obj_type() == OBJ_BOOL || obj_type() == OBJ_DATE ||
           obj_type() == OBJ_NUMBER || obj_type() == OBJ_STRING;
  }
  // [[Construct]]
  virtual Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) {
    assert(false);
  }
  bool IsConstructor() override { return READ_VALUE(this, kIsConstructorOffset, bool); }
  // [[Call]]
  virtual Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) {
    inner_func callable = TYPED_PTR(this, kCallableOffset, inner_func)[0];
    assert(IsCallable() && callable != nullptr);
    return callable(e, Handle<JSValue>(this), arguments);
  }
  bool IsCallable() override { return READ_VALUE(this, kIsCallableOffset, bool); }
  // [[HasInstance]]
  // NOTE(zhuzilin) Here we use the implementation in 15.3.5.3 [[HasInstance]] (V)
  // to make sure all callables have HasInstance.
  virtual bool HasInstance(Error* e, Handle<JSValue> V) {
    assert(IsCallable());
    if (!V.val()->IsObject())
      return false;
    Handle<JSValue> O = Get(e, String::Prototype());
    if (!e->IsOk()) return false;
    if (!O.val()->IsObject()) {
      *e = *Error::TypeError();
      return false;
    }
    while (!V.val()->IsNull()) {
      if (V.val() == O.val())
        return true;
      assert(V.val()->IsObject());
      V = static_cast<Handle<JSObject>>(V).val()->Prototype();
      if (!e->IsOk()) return false;
    }
    return false;
  }

  void AddValueProperty(
    std::u16string name, Handle<JSValue> value, bool writable,
    bool enumerable, bool configurable
  ) {
    return AddValueProperty(String::New(name), value, writable, enumerable, configurable);
  }

  void AddValueProperty(
    Handle<String> name, Handle<JSValue> value, bool writable,
    bool enumerable, bool configurable
  ) {
    Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
    desc.val()->SetDataDescriptor(value, writable, enumerable, configurable);
    // This should just like named_properties_[name] = desc
    DefineOwnProperty(nullptr, name, desc, false);
  }

  void AddFuncProperty(
    std::u16string name, inner_func callable, bool writable,
    bool enumerable, bool configurable
  );

  // This for for-in statement.
  virtual std::vector<std::pair<Handle<String>, Handle<PropertyDescriptor>>> AllEnumerableProperties() {
    auto filter = [](Handle<PropertyDescriptor> desc) {
      return desc.val()->HasEnumerable() && desc.val()->Enumerable();
    };
    auto result = named_properties().val()->SortedKeyValPairs(filter);
    if (!Prototype().val()->IsNull()) {
      Handle<JSObject> proto = static_cast<Handle<JSObject>>(Prototype());
      for (auto pair : proto.val()->AllEnumerableProperties()) {
        if (!pair.second.val()->HasEnumerable() || !pair.second.val()->Enumerable())
          continue;
        if (named_properties().val()->Get(pair.first).IsNullptr()) {
          result.emplace_back(pair);
        }
      }
    }
    return result;
  }

  virtual std::string ToString() override { return log::ToString(Class().val()->data()); }

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
  Handle<HashMap<PropertyDescriptor>> named_properties() { return READ_HANDLE_VALUE(this, kNamedPropertiesOffset, HashMap<PropertyDescriptor>); };
};

// 8.12.1 [[GetOwnProperty]] (P)
Handle<JSValue> JSObject::GetOwnProperty(Handle<String> P) {
  // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
  Handle<JSValue> val = named_properties().val()->Get(P);
  if (val.IsNullptr()) {
    return Undefined::Instance();
  }
  // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
  // And assign the property to it. However, if we init D->value = a, and
  // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
  // remain b and that is not what we want.
  return val;
}

Handle<JSValue> JSObject::GetProperty(Handle<String> P) {
  Handle<JSValue> own_property = GetOwnProperty(P);
  if (!own_property.val()->IsUndefined()) {
    return own_property;
  }
  Handle<JSValue> proto = Prototype();
  if (proto.val()->IsNull()) {
    return Undefined::Instance();
  }
  assert(proto.val()->IsObject());
  Handle<JSObject> proto_obj = static_cast<Handle<JSObject>>(proto);
  return proto_obj.val()->GetProperty(P);
}

Handle<JSValue> JSObject::Get(Error* e, Handle<String> P) {
  Handle<JSValue> value = GetProperty(P);
  if (value.val()->IsUndefined()) {
    return Undefined::Instance();
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
  if (desc.val()->IsDataDescriptor()) {
    return desc.val()->Value();
  } else {
    assert(desc.val()->IsAccessorDescriptor());
    Handle<JSValue> getter = desc.val()->Get();
    if (getter.val()->IsUndefined()) {
      return Undefined::Instance();
    }
    Handle<JSObject> getter_obj = static_cast<Handle<JSObject>>(getter);
    return getter_obj.val()->Call(e, Handle<JSValue>(this));
  }
}

bool JSObject::CanPut(Handle<String> P) {
  Handle<JSValue> value = GetOwnProperty(P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
    if (desc.val()->IsAccessorDescriptor()) {
      return !desc.val()->Set().val()->IsUndefined();
    } else {
      return desc.val()->Writable();
    }
  }

  Handle<JSValue> proto = Prototype();
  if (proto.val()->IsNull()) {
    return Extensible();
  }
  Handle<JSObject> proto_obj = static_cast<Handle<JSObject>>(proto);
  Handle<JSValue> inherit = proto_obj.val()->GetProperty(P);
  if (inherit.val()->IsUndefined()) {
    return Extensible();
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(inherit);
  if (desc.val()->IsAccessorDescriptor()) {
    return !desc.val()->Set().val()->IsUndefined();
  } else {
    return Extensible() ? desc.val()->Writable() : false;
  }
}

// 8.12.5 [[Put]] ( P, V, Throw )
void JSObject::Put(Error* e, Handle<String> P, Handle<JSValue> V, bool throw_flag) {
  assert(V.val()->IsLanguageType());
  if (!CanPut(P)) {  // 1
    if (throw_flag) {  // 1.a
      *e = *Error::TypeError();
    }
    return;  // 1.b
  }
  Handle<JSValue> value = GetOwnProperty(P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> own_desc = static_cast<Handle<PropertyDescriptor>>(value);  // 2
    if (own_desc.val()->IsDataDescriptor()) {  // 3
      Handle<PropertyDescriptor> value_desc = PropertyDescriptor::New();
      value_desc.val()->SetValue(V);
      log::PrintSource("Overwrite the old desc with " + value_desc.ToString());
      DefineOwnProperty(e, P, value_desc, throw_flag);
      return;
    }
  }
  value = GetProperty(P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
    if (desc.val()->IsAccessorDescriptor()) {
      log::PrintSource("Use parent prototype's setter");
      Handle<JSValue> setter = desc.val()->Set();
      assert(!setter.val()->IsUndefined());
      Handle<JSObject> setter_obj = static_cast<Handle<JSObject>>(setter);
      setter_obj.val()->Call(e, Handle<JSValue>(this), {V});
      return;
    }
  }
  Handle<PropertyDescriptor> new_desc = PropertyDescriptor::New();
  new_desc.val()->SetDataDescriptor(V, true, true, true);  // 6.a
  DefineOwnProperty(e, P, new_desc, throw_flag);
}

bool JSObject::HasProperty(Handle<String> P) {
  Handle<JSValue> desc = GetOwnProperty(P);
  return !desc.val()->IsUndefined();
}

bool JSObject::Delete(Error* e, Handle<String> P, bool throw_flag) {
  Handle<JSValue> value = GetOwnProperty(P);
  if (value.val()->IsUndefined()) {
    return true;
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
  if (desc.val()->Configurable()) {
    named_properties().val()->Delete(P);
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
  Error* e, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  Handle<JSValue> current = GetOwnProperty(P);
  Handle<PropertyDescriptor> current_desc;
  if (current.val()->IsUndefined()) {
    if(!Extensible())  // 3
      goto reject;
    // 4.
    named_properties().val()->Set(P, desc);
    return true;
  }
  if (desc.val()->bitmask() == 0) {  // 5
    return true;
  }
  current_desc = static_cast<Handle<PropertyDescriptor>>(current);
  if ((desc.val()->bitmask() & current_desc.val()->bitmask()) == desc.val()->bitmask()) {
    bool same = true;
    if (desc.val()->HasValue())
      same = same && SameValue(desc.val()->Value(), current_desc.val()->Value());
    if (desc.val()->HasWritable())
      same = same && (desc.val()->Writable() == current_desc.val()->Writable());
    if (desc.val()->HasGet())
      same = same && SameValue(desc.val()->Get(), current_desc.val()->Get());
    if (desc.val()->HasSet())
      same = same && SameValue(desc.val()->Set(), current_desc.val()->Set());
    if (desc.val()->HasConfigurable())
      same = same && (desc.val()->Configurable() == current_desc.val()->Configurable());
    if (desc.val()->HasEnumerable())
      same = same && (desc.val()->Enumerable() == current_desc.val()->Enumerable());
    if (same) return true;  // 6
  }
  log::PrintSource("desc: " + desc.ToString() + ", current: " + current_desc.ToString());
  if (!current_desc.val()->Configurable()) { // 7
    if (desc.val()->Configurable()) {  // 7.a
      log::PrintSource("DefineOwnProperty: " + P.ToString() + " not configurable, while new value configurable");
      goto reject;
    }
    if (desc.val()->HasEnumerable() && (desc.val()->Enumerable() != current_desc.val()->Enumerable())) {  // 7.b
      log::PrintSource("DefineOwnProperty: " + P.ToString() + " enumerable value differ");
      goto reject;
    }
  }
  // 8.
  if (!desc.val()->IsGenericDescriptor()) {
    if (current_desc.val()->IsDataDescriptor() != desc.val()->IsDataDescriptor()) {  // 9.
      // 9.a
      if (!current_desc.val()->Configurable()) goto reject;
      // 9.b.i & 9.c.i
      Handle<PropertyDescriptor> old_property = named_properties().val()->Get(P);
      Handle<PropertyDescriptor> new_property = PropertyDescriptor::New();
      new_property.val()->SetConfigurable(old_property.val()->Configurable());
      new_property.val()->SetEnumerable(old_property.val()->Enumerable());
      new_property.val()->SetBitMask(old_property.val()->bitmask());
      named_properties().val()->Set(P, new_property);
    } else if (current_desc.val()->IsDataDescriptor() && desc.val()->IsDataDescriptor()) {  // 10.
      if (!current_desc.val()->Configurable()) {  // 10.a
        if (!current_desc.val()->Writable()) {
          if (desc.val()->Writable()) goto reject;  // 10.a.i
          // 10.a.ii.1
          if (desc.val()->HasValue() && !SameValue(desc.val()->Value(), current_desc.val()->Value())) goto reject;
        }
      } else {  // 10.b
        assert(current_desc.val()->Configurable());
      }
    } else {  // 11.
      assert(current_desc.val()->IsAccessorDescriptor() && desc.val()->IsAccessorDescriptor());
      if (!current_desc.val()->Configurable()) {  // 11.a
        if (!SameValue(desc.val()->Set(), current_desc.val()->Set()) ||  // 11.a.i
            !SameValue(desc.val()->Get(), current_desc.val()->Get()))    // 11.a.ii
          goto reject;
      }
    }
  }
  log::PrintSource("DefineOwnProperty: " + P.ToString() + " is set to " + desc.val()->Value().ToString());
  // 12.
  current_desc.val()->Set(desc);
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