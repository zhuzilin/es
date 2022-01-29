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

    OBJ_FUNC_PROTO,

    OBJ_ARRAY_CONSTRUCTOR,
    OBJ_BOOL_CONSTRUCTOR,
    OBJ_DATE_CONSTRUCTOR,
    OBJ_ERROR_CONSTRUCTOR,
    OBJ_FUNC_CONSTRUCTOR,
    OBJ_NUMBER_CONSTRUCTOR,
    OBJ_OBJECT_CONSTRUCTOR,
    OBJ_REGEXP_CONSTRUCTOR,
    OBJ_STRING_CONSTRUCTOR,

    OBJ_ARGUMENTS,

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
    size_t size,
    flag_t flag = 0
  ) {
#ifdef GC_DEBUG
    if (log::Debugger::On())
      std::cout << "JSObject::New " << log::ToString(klass) << std::endl;
#endif
    Handle<JSValue> jsval = JSValue::New(JS_OBJECT, kJSObjectOffset - kJSValueOffset + size, flag);
    // NOTE(zhuzilin) We need to put the operation that may need memory allocation to
    // the front, because the jsval is not initialized with JSObject vptr and therefore
    // could not forward the pointers.
    auto class_str = String::New(klass);
    auto property_map = HashMap<PropertyDescriptor>::New();

    SET_VALUE(jsval.val(), kObjTypeOffset, obj_type, ObjType);
    SET_HANDLE_VALUE(jsval.val(), kClassOffset, class_str, String);
    SET_VALUE(jsval.val(), kExtensibleOffset, extensible, bool);
    SET_HANDLE_VALUE(jsval.val(), kPrimitiveValueOffset, primitive_value, JSValue);
    SET_VALUE(jsval.val(), kIsConstructorOffset, is_constructor, bool);
    SET_VALUE(jsval.val(), kIsCallableOffset, is_callable, bool);
    // NOTE(zhuzilin) function pointer is different.
    TYPED_PTR(jsval.val(), kCallableOffset, inner_func)[0] = callable;
    SET_HANDLE_VALUE(jsval.val(), kPrototypeOffset, Null::Instance(), JSValue);
    SET_HANDLE_VALUE(jsval.val(), kNamedPropertiesOffset, property_map, HashMap<PropertyDescriptor>);

    new (jsval.val()) JSObject();
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
  HashMap<PropertyDescriptor>* named_properties() {
    return READ_VALUE(this, kNamedPropertiesOffset, HashMap<PropertyDescriptor>*);
  };
  bool IsFunction() { return obj_type() == OBJ_FUNC; }

  // Internal Preperties Common to All Objects
  Handle<JSValue> Prototype() { return READ_HANDLE_VALUE(this, kPrototypeOffset, JSValue); }
  void SetPrototype(Handle<JSValue> proto) {
    assert(proto.val()->IsPrototype());
    SET_HANDLE_VALUE(this, kPrototypeOffset, proto, JSValue);
  }
  std::u16string Class() { return (READ_VALUE(this, kClassOffset, String*))->data(); };
  bool Extensible() { return READ_VALUE(this, kExtensibleOffset, bool); };
  void SetExtensible(bool extensible) { SET_VALUE(this, kExtensibleOffset, extensible, bool); }
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
  bool IsConstructor() override { return READ_VALUE(this, kIsConstructorOffset, bool); }
  bool IsCallable() override { return READ_VALUE(this, kIsCallableOffset, bool); }
  inner_func callable() { return TYPED_PTR(this, kCallableOffset, inner_func)[0]; }

  // This for for-in statement.
  std::vector<std::pair<Handle<String>, Handle<PropertyDescriptor>>> AllEnumerableProperties() {
    auto filter = [](PropertyDescriptor* desc) {
      return desc->HasEnumerable() && desc->Enumerable();
    };
    std::vector<std::pair<Handle<String>, Handle<PropertyDescriptor>>> result;
    for (auto pair : named_properties()->SortedKeyValPairs(filter)) {
      result.emplace_back(std::make_pair(Handle<String>(pair.first), Handle<PropertyDescriptor>(pair.second)));
    }
    if (!Prototype().val()->IsNull()) {
      Handle<JSObject> proto = static_cast<Handle<JSObject>>(Prototype());
      for (auto pair : proto.val()->AllEnumerableProperties()) {
        if (named_properties()->GetRaw(pair.first) == nullptr) {
          result.emplace_back(pair);
        }
      }
    }
    return result;
  }

  virtual std::string ToString() override { return (READ_VALUE(this, kClassOffset, String*))->ToString(); }

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
};

bool Is(JSValue* val, JSObject::ObjType obj_type) {
  return val->IsObject() ? (static_cast<JSObject*>(val)->obj_type() == obj_type) : false;
}
bool JSValue::IsNumberObject() { return Is(this, JSObject::OBJ_NUMBER); }
bool JSValue::IsArrayObject() { return Is(this, JSObject::OBJ_ARRAY); }
bool JSValue::IsRegExpObject() { return Is(this, JSObject::OBJ_REGEXP); }
bool JSValue::IsErrorObject() { return Is(this, JSObject::OBJ_ERROR); }
bool JSValue::IsFunctionObject() { return Is(this, JSObject::OBJ_FUNC); }
bool JSValue::IsStringObject() { return Is(this, JSObject::OBJ_STRING); }
bool JSValue::IsDateObject() { return Is(this, JSObject::OBJ_DATE); }
bool JSValue::IsArgumentsObject() { return Is(this, JSObject::OBJ_ARGUMENTS); }

bool JSValue::IsFunctionProto() { return Is(this, JSObject::OBJ_FUNC_PROTO); }

bool JSValue::IsBoolConstructor() { return Is(this, JSObject::OBJ_BOOL_CONSTRUCTOR); }
bool JSValue::IsNumberConstructor() { return Is(this, JSObject::OBJ_NUMBER_CONSTRUCTOR); }
bool JSValue::IsObjectConstructor() { return Is(this, JSObject::OBJ_OBJECT_CONSTRUCTOR); }
bool JSValue::IsRegExpConstructor() { return Is(this, JSObject::OBJ_REGEXP_CONSTRUCTOR); }
bool JSValue::IsStringConstructor() { return Is(this, JSObject::OBJ_STRING_CONSTRUCTOR); }

Handle<JSValue> Get(Error* e, Handle<JSObject> O, Handle<String> P);
Handle<JSValue> Get__Base(Error* e, Handle<JSObject> O, Handle<String> P);
Handle<JSValue> GetOwnProperty(Handle<JSObject> O, Handle<String> P);
Handle<JSValue> GetOwnProperty__Base(Handle<JSObject> O, Handle<String> P);
Handle<JSValue> GetProperty(Handle<JSObject> O, Handle<String> P);
void Put(Error* e, Handle<JSObject> O, Handle<String> P, Handle<JSValue> V, bool throw_flag);
bool CanPut(Handle<JSObject> O, Handle<String> P);
bool HasProperty(Handle<JSObject> O, Handle<String> P);
bool Delete(Error* e, Handle<JSObject> O, Handle<String> P, bool throw_flag);
bool Delete__Base(Error* e, Handle<JSObject> O, Handle<String> P, bool throw_flag);
Handle<JSValue> DefaultValue(Error* e, Handle<JSObject> O, std::u16string hint);
bool DefineOwnProperty(Error* e, Handle<JSObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag);
bool DefineOwnProperty__Base(Error* e, Handle<JSObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag);
bool HasInstance(Error* e, Handle<JSObject> O, Handle<JSValue> V);
bool HasInstance__Base(Error* e, Handle<JSObject> O, Handle<JSValue> V);

void AddValueProperty(
  Handle<JSObject> O, Handle<String> name, Handle<JSValue> value, bool writable,
  bool enumerable, bool configurable
);

void AddValueProperty(
  Handle<JSObject> O, std::u16string name, Handle<JSValue> value, bool writable,
  bool enumerable, bool configurable
) {
  return AddValueProperty(O, String::New(name), value, writable, enumerable, configurable);
}

void AddFuncProperty(
  Handle<JSObject> O, std::u16string name, inner_func callable, bool writable,
  bool enumerable, bool configurable
);

Handle<JSValue> Call(Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSValue> Call__Base(Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSValue> Call__Construct(Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});

Handle<JSObject> Construct(Error* e, Handle<JSObject> O, std::vector<Handle<JSValue>> arguments);

}  // namespace

#endif  // ES_OBJECT_H