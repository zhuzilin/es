#ifndef ES_TYPES_BUILTIN_ARGUMENTS_OBJECT
#define ES_TYPES_BUILTIN_ARGUMENTS_OBJECT

#include <iostream>

#include <es/types/object.h>
#include <es/types/builtin/function_object.h>
#include <es/utils/helper.h>

namespace es {

// 10.6 Arguments Object
class ArgumentsObject : public JSObject {
 public:
  static Handle<ArgumentsObject> New(size_t len) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Arguments", true, Handle<JSValue>(), false, false, nullptr, 0
    );

    jsobj.val()->SetType(OBJ_ARGUMENTS);

    Handle<ArgumentsObject> obj(jsobj);
    obj.val()->SetPrototype(ObjectProto::Instance());
    AddValueProperty(obj, String::Length(), Number::New(len), true, false, true);
    return obj;
  }
};

Handle<JSValue> Get__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P);
Handle<JSValue> GetOwnProperty__Arguments(Handle<ArgumentsObject> O, Handle<String> P);
bool Delete__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P, bool throw_flag);
bool DefineOwnProperty__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag);

Handle<JSValue> Call__GetterSetter(Handle<Error>& e, Handle<Reference> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARGUMENTS_OBJECT