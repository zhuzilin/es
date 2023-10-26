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
#include <es/types/error.h>
#include <es/utils/helper.h>
#include <es/utils/hashmap.h>

namespace es {

typedef JSValue (*inner_func)(JSValue&, JSValue, std::vector<JSValue>);

namespace js_object {

constexpr size_t kClassOffset = 0;
constexpr size_t kExtensibleOffset = kClassOffset + sizeof(JSValue);
constexpr size_t kPrimitiveValueOffset = kExtensibleOffset + kBoolSize;
constexpr size_t kIsConstructorOffset = kPrimitiveValueOffset + sizeof(JSValue);
constexpr size_t kIsCallableOffset = kIsConstructorOffset + kBoolSize;
constexpr size_t kCallableOffset = kIsCallableOffset + kBoolSize;
constexpr size_t kPrototypeOffset = kCallableOffset + kFuncPtrSize;
constexpr size_t kNamedPropertiesOffset = kPrototypeOffset + sizeof(JSValue);
constexpr size_t kJSObjectOffset = kNamedPropertiesOffset + sizeof(JSValue);

inline JSValue New(
  std::u16string klass,
  bool extensible,
  JSValue primitive_value,
  bool is_constructor,
  bool is_callable,
  inner_func callable,
  size_t size,
  flag_t flag = 0
) {
  JSValue jsval;
  jsval.handle() = HeapObject::New(kJSObjectOffset + size, flag);
  // NOTE(zhuzilin) We need to put the operation that may need memory allocation to
  // the front, because the jsval is not initialized with JSObject vptr and therefore
  // could not forward the pointers.
  JSValue class_str = string::New(klass);
  JSValue property_map = hash_map::New();

  SET_JSVALUE(jsval.handle().val(), kClassOffset, class_str);
  SET_VALUE(jsval.handle().val(), kExtensibleOffset, extensible, bool);
  SET_JSVALUE(jsval.handle().val(), kPrimitiveValueOffset, primitive_value);
  SET_VALUE(jsval.handle().val(), kIsConstructorOffset, is_constructor, bool);
  SET_VALUE(jsval.handle().val(), kIsCallableOffset, is_callable, bool);
  // NOTE(zhuzilin) function pointer is different.
  TYPED_PTR(jsval.handle().val(), kCallableOffset, inner_func)[0] = callable;
  SET_JSVALUE(jsval.handle().val(), kPrototypeOffset, null::New());
  SET_JSVALUE(jsval.handle().val(), kNamedPropertiesOffset, property_map);

  jsval.SetType(JS_OBJECT);
  return jsval;
}

JSValue& named_properties(JSValue obj) {
  return GET_JSVALUE(obj.handle().val(), kNamedPropertiesOffset);
};
void SetNamedProperties(JSValue& obj, JSValue new_named_properties) {
  SET_JSVALUE(obj.handle().val(), kNamedPropertiesOffset, new_named_properties);
}

// Internal Preperties Common to All Objects
JSValue Prototype(JSValue obj) { return GET_JSVALUE(obj.handle().val(), kPrototypeOffset); }
void SetPrototype(JSValue& obj, JSValue proto) {
  ASSERT(proto.IsPrototype());
  SET_JSVALUE(obj.handle().val(), kPrototypeOffset, proto);
}
std::u16string Class(JSValue obj) {
  return string::data(GET_JSVALUE(obj.handle().val(), kClassOffset));
};
bool Extensible(JSValue obj) {
  return READ_VALUE(obj.handle().val(), kExtensibleOffset, bool);
};
void SetExtensible(JSValue& obj, bool extensible) {
  SET_VALUE(obj.handle().val(), kExtensibleOffset, extensible, bool);
}
// Internal Properties Only Defined for Some Objects
// [[PrimitiveValue]]
JSValue PrimitiveValue(JSValue obj) {
  JSValue primitive_value = GET_JSVALUE(obj.handle().val(), kPrimitiveValueOffset);
  return primitive_value;
};
bool HasPrimitiveValue(JSValue obj) {
  return obj.type() == OBJ_BOOL || obj.type() == OBJ_DATE ||
         obj.type() == OBJ_NUMBER || obj.type() == OBJ_STRING;
}
inner_func callable(JSValue obj) {
  return TYPED_PTR(obj.handle().val(), kCallableOffset, inner_func)[0];
}

// This for for-in statement.
std::vector<std::pair<JSValue, JSValue>> AllEnumerableProperties(JSValue obj) {
  auto filter = [](JSValue desc) {
    ASSERT(desc.IsPropertyDescriptor());
    return property_descriptor::HasEnumerable(desc) && property_descriptor::Enumerable(desc);
  };
  std::vector<std::pair<JSValue, JSValue>> result;
  for (auto pair : hash_map::SortedKeyValPairs(named_properties(obj), filter)) {
    result.emplace_back(std::make_pair(pair.first, pair.second));
  }
  JSValue proto = Prototype(obj);
  if (!proto.IsNull()) {
    ASSERT(obj.IsObject());
    for (auto pair : AllEnumerableProperties(proto)) {
      if (hash_map::Get(named_properties(obj), pair.first).IsNull()) {
        result.emplace_back(pair);
      }
    }
  }
  return result;
}

}  // namespace js_object

JSValue Get(JSValue& e, JSValue O, JSValue P);
JSValue Get__Base(JSValue& e, JSValue O, JSValue P);
JSValue GetOwnProperty(JSValue O, JSValue P);
JSValue GetOwnProperty__Base(JSValue O, JSValue P);
JSValue GetProperty(JSValue O, JSValue P);
void Put(JSValue& e, JSValue O, JSValue P, JSValue V, bool throw_flag);
bool CanPut(JSValue O, JSValue P);
bool HasProperty(JSValue O, JSValue P);
bool Delete(JSValue& e, JSValue O, JSValue P, bool throw_flag);
bool Delete__Base(JSValue& e, JSValue O, JSValue P, bool throw_flag);
JSValue DefaultValue(JSValue& e, JSValue O, std::u16string hint);
bool DefineOwnProperty(JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag);
bool DefineOwnProperty__Base(JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag);
bool HasInstance(JSValue& e, JSValue O, JSValue V);
bool HasInstance__Base(JSValue& e, JSValue O, JSValue V);

void AddValueProperty(
  JSValue O, JSValue name, JSValue value, bool writable,
  bool enumerable, bool configurable
);

inline void AddValueProperty(
  JSValue O, std::u16string name, JSValue value, bool writable,
  bool enumerable, bool configurable
) {
  return AddValueProperty(O, string::New(name), value, writable, enumerable, configurable);
}

inline void AddFuncProperty(
  JSValue O, std::u16string name, inner_func callable, bool writable,
  bool enumerable, bool configurable
);

JSValue Call(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Call__Base(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Call__Construct(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});

JSValue Construct(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace

#endif  // ES_OBJECT_H