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
  static Handle<ArgumentsObject> New(Handle<JSObject> parameter_map, size_t len) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_ARGUMENTS, u"Arguments", true, Handle<JSValue>(), false, false, nullptr,
      kParameterMapOffset + kPtrSize - kJSObjectOffset
    );

    SET_HANDLE_VALUE(jsobj.val(), kParameterMapOffset, parameter_map, JSObject);

    Handle<ArgumentsObject> obj(new (jsobj.val()) ArgumentsObject());
    obj.val()->SetPrototype(ObjectProto::Instance());
    AddValueProperty(obj, String::Length(), Number::New(len), true, false, true);
    return obj;
  }

  std::vector<HeapObject**> Pointers() override {
    std::vector<HeapObject**> pointers = JSObject::Pointers();
    pointers.emplace_back(HEAP_PTR(kParameterMapOffset));
    return pointers;
  }

  Handle<JSObject> ParameterMap() { return READ_HANDLE_VALUE(this, kParameterMapOffset, JSObject); }

  inline std::string ToString() override { return "ArgumentsObject"; }

 private:
  static constexpr size_t kParameterMapOffset = kJSObjectOffset;
};

Handle<JSValue> Get__Arguments(Error* e, Handle<ArgumentsObject> O, Handle<String> P);
Handle<JSValue> GetOwnProperty__Arguments(Handle<ArgumentsObject> O, Handle<String> P);
bool Delete__Arguments(Error* e, Handle<ArgumentsObject> O, Handle<String> P, bool throw_flag);
bool DefineOwnProperty__Arguments(Error* e, Handle<ArgumentsObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARGUMENTS_OBJECT