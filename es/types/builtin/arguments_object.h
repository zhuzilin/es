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
    Handle<JSObject> jsobj = JSObject::New<0, 0>(
      CLASS_ARGUMENTS, true, Handle<JSValue>(), false, false, nullptr, len
    );

    jsobj.val()->SetType(OBJ_ARGUMENTS);

    Handle<ArgumentsObject> obj(jsobj);
    obj.val()->SetPrototype(ObjectProto::Instance());
    AddValueProperty(obj, String::Length(), Number::New(len), true, false, true);
    return obj;
  }
};

class GetterSetter : public JSValue {
 public:
  static Handle<Reference> New(
    Handle<JSValue> base,
    Handle<String> reference_name,
    bool strict_reference
  ) {
    Handle<JSValue> jsval = HeapObject::New<kStrictOffset + kBoolSize - kJSValueOffset>();

    SET_HANDLE_VALUE(jsval.val(), kBaseOffset, base, JSValue);
    SET_HANDLE_VALUE(jsval.val(), kReferenceNameOffset, reference_name, String);
    SET_VALUE(jsval.val(), kStrictOffset, strict_reference, bool);

    jsval.val()->SetType(JS_GET_SET);
    return Handle<Reference>(jsval);
  }

  Handle<JSValue> GetBase() { return READ_HANDLE_VALUE(this, kBaseOffset, JSValue); }
  Handle<String> GetReferencedName() { return READ_HANDLE_VALUE(this, kReferenceNameOffset, String); }
  bool IsStrictReference() { return READ_VALUE(this, kStrictOffset, bool); }

 public:
  static constexpr size_t kBaseOffset = kJSValueOffset;
  static constexpr size_t kReferenceNameOffset = kBaseOffset + kPtrSize;
  static constexpr size_t kStrictOffset = kReferenceNameOffset + kPtrSize;
};

Handle<JSValue> Get__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P);
StackPropertyDescriptor GetOwnProperty__Arguments(Handle<ArgumentsObject> O, Handle<String> P);
bool Delete__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P, bool throw_flag);
bool DefineOwnProperty__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P, StackPropertyDescriptor desc, bool throw_flag);

Handle<JSValue> Call__GetterSetter(Handle<Error>& e, Handle<GetterSetter> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARGUMENTS_OBJECT