#ifndef ES_TYPES_BUILTIN_ARGUMENTS_OBJECT
#define ES_TYPES_BUILTIN_ARGUMENTS_OBJECT

#include <iostream>

#include <es/types/object.h>
#include <es/types/builtin/function_object.h>
#include <es/utils/helper.h>

namespace es {

// 10.6 Arguments Object
namespace arguments_object {

inline JSValue New(size_t len) {
  JSValue jsobj = js_object::New(
    u"Arguments", true, JSValue(), false, false, nullptr, 0
  );

  jsobj.SetType(OBJ_ARGUMENTS);

  js_object::SetPrototype(jsobj, object_proto::Instance());
  AddValueProperty(jsobj, string::Length(), number::New(len), true, false, true);
  return jsobj;
}

}  // namespace arguments_object

namespace getter_setter {

constexpr size_t kReferenceOffset = 0;

static JSValue New(JSValue ref) {
  JSValue jsval;
  std::cout << "enter gs" << std::endl;
  jsval.handle() = HeapObject::New(sizeof(JSValue), 0);

  SET_JSVALUE(jsval.handle().val(), kReferenceOffset, ref);

  jsval.SetType(JS_GET_SET);
  return jsval;
}

JSValue ref(JSValue gs) { return GET_JSVALUE(gs.handle().val(), kReferenceOffset); }

}  // namespace getter_setter

JSValue Get__Arguments(JSValue& e, JSValue O, JSValue P);
JSValue GetOwnProperty__Arguments(JSValue O, JSValue P);
bool Delete__Arguments(JSValue& e, JSValue O, JSValue P, bool throw_flag);
bool DefineOwnProperty__Arguments(JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag);

JSValue Call__GetterSetter(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARGUMENTS_OBJECT