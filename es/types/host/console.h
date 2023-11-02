#ifndef ES_TYPES_HOST_CONSOLE
#define ES_TYPES_HOST_CONSOLE

#include <iostream>
#include <es/types/object.h>

namespace es {

bool ToBoolean(Handle<JSValue>);

class Console : public JSObject {
 public:
  static  Handle<Console> Instance() {
    static Handle<Console> singleton = Console::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> log(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    std::vector<std::string> strs;
    for (size_t i = 0; i < vals.size(); ++i) {
      if (i != 0)
        strs.emplace_back(" ");
      strs.emplace_back(vals[i].ToString());
    }
    strs.emplace_back("\n");
    std::cout << StrCat(strs);
    return Number::Zero();
  }

 private:
  template<flag_t flag>
  static Handle<Console> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_CONSOLE, true, Handle<JSValue>(), false, false, nullptr
    );

    jsobj.val()->SetType(OBJ_HOST);
    Handle<Console> obj(jsobj);
    AddFuncProperty(obj, String::New<GCFlag::CONST>(u"log"), log, false, false, false);
    return obj;
  }
};

}  // namespace es

#endif  // ES_TYPES_HOST_CONSOLE