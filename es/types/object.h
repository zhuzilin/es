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
#include <es/utils/property_map.h>

namespace es {

typedef Handle<JSValue> (*inner_func)(Handle<Error>&, Handle<JSValue>, std::vector<Handle<JSValue>>);

class JSObject : public JSValue {
 public:
  template<flag_t flag = 0>
  static Handle<JSObject> New(
    ClassType klass,
    bool extensible,
    Handle<JSValue> primitive_value,
    bool is_constructor,
    bool is_callable,
    inner_func callable,
    size_t size,
    size_t property_map_num_fixed_slots = 0,
    size_t property_map_hashmap_capacity = 0
  ) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "JSObject::New " << log::ToString(klass) << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New<flag>(kJSObjectOffset - kJSValueOffset + size);
    // NOTE(zhuzilin) We need to put the operation that may need memory allocation to
    // the front, because the jsval is not initialized with JSObject vptr and therefore
    // could not forward the pointers.
    auto property_map = PropertyMap::New(property_map_num_fixed_slots, property_map_hashmap_capacity);

    jsval.val()->h_.klass = klass;
    jsval.val()->h_.extensible = extensible;
    jsval.val()->h_.is_constructor = is_constructor;
    jsval.val()->h_.is_callable = is_callable;

    ASSERT(!(callable != nullptr && !primitive_value.IsNullptr()));
    if (!primitive_value.IsNullptr()) {
      SET_HANDLE_VALUE(jsval.val(), kPrimitiveValueOffset, primitive_value, JSValue);
    } else {
      // NOTE(zhuzilin) function pointer is different.
      TYPED_PTR(jsval.val(), kCallableOffset, inner_func)[0] = callable;
    }
    SET_HANDLE_VALUE(jsval.val(), kPrototypeOffset, Null::Instance(), JSValue);
    SET_HANDLE_VALUE(jsval.val(), kNamedPropertiesOffset, property_map, PropertyMap);

    jsval.val()->SetType(JS_OBJECT);
    return Handle<JSObject>(jsval);
  }

  template<size_t size, flag_t flag = 0>
  static Handle<JSObject> New(
    ClassType klass,
    bool extensible,
    Handle<JSValue> primitive_value,
    bool is_constructor,
    bool is_callable,
    inner_func callable,
    size_t property_map_num_fixed_slots = 0,
    size_t property_map_hashmap_capacity = 0
  ) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "JSObject::New " << log::ToString(klass) << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New<kJSObjectOffset - kJSValueOffset + size, flag>();
    // NOTE(zhuzilin) We need to put the operation that may need memory allocation to
    // the front, because the jsval is not initialized with JSObject vptr and therefore
    // could not forward the pointers.
    auto property_map = PropertyMap::New(property_map_num_fixed_slots, property_map_hashmap_capacity);

    jsval.val()->h_.klass = klass;
    jsval.val()->h_.extensible = extensible;
    jsval.val()->h_.is_constructor = is_constructor;
    jsval.val()->h_.is_callable = is_callable;

    ASSERT(!(callable != nullptr && !primitive_value.IsNullptr()));
    if (!primitive_value.IsNullptr()) {
      SET_HANDLE_VALUE(jsval.val(), kPrimitiveValueOffset, primitive_value, JSValue);
    } else {
      // NOTE(zhuzilin) function pointer is different.
      TYPED_PTR(jsval.val(), kCallableOffset, inner_func)[0] = callable;
    }
    SET_HANDLE_VALUE(jsval.val(), kPrototypeOffset, Null::Instance(), JSValue);
    SET_HANDLE_VALUE(jsval.val(), kNamedPropertiesOffset, property_map, PropertyMap);

    jsval.val()->SetType(JS_OBJECT);
    return Handle<JSObject>(jsval);
  }

  PropertyMap* named_properties() {
    return READ_VALUE(this, kNamedPropertiesOffset, PropertyMap*);
  };

  // Internal Preperties Common to All Objects
  Handle<JSValue> Prototype() { return READ_HANDLE_VALUE(this, kPrototypeOffset, JSValue); }
  void SetPrototype(Handle<JSValue> proto) {
    ASSERT(proto.val()->IsPrototype());
    SET_HANDLE_VALUE(this, kPrototypeOffset, proto, JSValue);
  }
  ClassType Class() { return h_.klass; };
  bool Extensible() { return h_.extensible; };
  void SetExtensible(bool extensible) { h_.extensible = extensible; }
  // Internal Properties Only Defined for Some Objects
  // [[PrimitiveValue]]
  Handle<JSValue> PrimitiveValue() {
    ASSERT(!IsCallable());
    Handle<JSValue> primitive_value = READ_HANDLE_VALUE(this, kPrimitiveValueOffset, JSValue);
    ASSERT(!primitive_value.IsNullptr());
    return primitive_value;
  };
  bool HasPrimitiveValue() {
    return type() == OBJ_BOOL || type() == OBJ_DATE ||
           type() == OBJ_NUMBER || type() == OBJ_STRING;
  }
  inner_func callable() {
    ASSERT(!HasPrimitiveValue());
    return TYPED_PTR(this, kCallableOffset, inner_func)[0];
  }

  // This for for-in statement.
  std::vector<std::pair<Handle<String>, StackPropertyDescriptor>> AllEnumerableProperties() {
    auto filter = [](StackPropertyDescriptor desc) {
      return desc.HasEnumerable() && desc.Enumerable();
    };
    std::vector<std::pair<Handle<String>, StackPropertyDescriptor>> result;
    for (auto pair : named_properties()->SortedKeyValPairs(filter)) {
      result.emplace_back(std::make_pair(Handle<String>(pair.first), pair.second));
    }
    if (!Prototype().val()->IsNull()) {
      Handle<JSObject> proto = static_cast<Handle<JSObject>>(Prototype());
      for (auto pair : proto.val()->AllEnumerableProperties()) {
        if (named_properties()->Get(pair.first).IsUndefined()) {
          result.emplace_back(pair);
        }
      }
    }
    return result;
  }

 public:
  // primitive value and offset are saved together.
  static constexpr size_t kPrimitiveValueOffset = kJSValueOffset;
  static constexpr size_t kCallableOffset = kJSValueOffset;

  static constexpr size_t kPrototypeOffset = kCallableOffset + kFuncPtrSize;
  static constexpr size_t kNamedPropertiesOffset = kPrototypeOffset + kPtrSize;
  static constexpr size_t kJSObjectOffset = kNamedPropertiesOffset + kPtrSize;
};

Handle<JSValue> Get(Handle<Error>& e, Handle<JSObject> O, Handle<String> P);
Handle<JSValue> Get__Base(Handle<Error>& e, Handle<JSObject> O, Handle<String> P);
StackPropertyDescriptor GetOwnProperty(Handle<JSObject> O, Handle<String> P);
StackPropertyDescriptor GetOwnProperty__Base(Handle<JSObject> O, Handle<String> P);
StackPropertyDescriptor GetProperty(Handle<JSObject> O, Handle<String> P);
void Put(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, Handle<JSValue> V, bool throw_flag);
bool CanPut(Handle<JSObject> O, Handle<String> P);
bool HasProperty(Handle<JSObject> O, Handle<String> P);
bool Delete(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, bool throw_flag);
bool Delete__Base(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, bool throw_flag);
template<Type hint>
Handle<JSValue> DefaultValue(Handle<Error>& e, Handle<JSObject> O);
bool DefineOwnProperty(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, StackPropertyDescriptor desc, bool throw_flag);
bool DefineOwnProperty__Base(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, StackPropertyDescriptor desc, bool throw_flag);
bool HasInstance(Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> V);
bool HasInstance__Base(Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> V);

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
  Handle<JSObject> O, Handle<String> name, inner_func callable, bool writable,
  bool enumerable, bool configurable
);

Handle<JSValue> Call(Handle<Error>& e, Handle<JSValue> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSValue> Call__Base(Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSValue> Call__Construct(Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});

Handle<JSObject> Construct(Handle<Error>& e, Handle<JSObject> O, std::vector<Handle<JSValue>> arguments);

}  // namespace

#endif  // ES_OBJECT_H