#ifndef ES_TYPES_HOST_CONSOLE
#define ES_TYPES_HOST_CONSOLE

#include <iostream>
#include <es/types/object.h>

namespace es {

bool ToBoolean(Handle<JSValue>);

class Console : public JSObject {
 public:
  static  Handle<Console> Instance() {
    static Handle<Console> singleton = Console::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> log(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size())
      std::cout << vals[0].ToString();
    for (size_t i = 1; i < vals.size(); i++) {
      std::cout << " " << vals[i].ToString();
    }
    std::cout << std::endl;
    return Number::Zero();
  }

 private:
  static Handle<Console> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_HOST, u"Console", true, Handle<JSValue>(), false, false, nullptr, 0, flag
    );

    new (jsobj.val()) Console();
    Handle<Console> obj(jsobj);
    AddFuncProperty(obj, u"log", log, false, false, false);
    return obj;
  }
};

}  // namespace es

#endif  // ES_TYPES_HOST_CONSOLE