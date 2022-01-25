#ifndef ES_TYPES_HOST_CONSOLE
#define ES_TYPES_HOST_CONSOLE

#include <iostream>
#include <es/types/object.h>

namespace es {

bool ToBoolean(JSValue*);

class Console : public JSObject {
 public:
  static  Console* Instance() {
    static Console* singleton = new Console();
    return singleton;
  }

  static JSValue* log(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size())
      std::cout << vals[0]->ToString();
    for (size_t i = 1; i < vals.size(); i++) {
      std::cout << " " << vals[i]->ToString();
    }
    std::cout << std::endl;
    return Number::Zero();
  }

 private:
   Console() :
    JSObject(
      OBJ_HOST, u"Console", true, nullptr, false, false
    ) {
      AddFuncProperty(u"log", log, false, false, false);
    }
};

}  // namespace es

#endif  // ES_TYPES_HOST_CONSOLE