#ifndef ES_TYPES_HOST_CONSOLE
#define ES_TYPES_HOST_CONSOLE

#include <iostream>
#include <es/types/object.h>

namespace es {

bool ToBoolean(JSValue*);

class Console : public JSObject {
 public:
  static  Console* Instance() {
    static  Console singleton;
    return &singleton;
  }

  static JSValue* log(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    for (auto val : vals) {
      std::cout << val->ToString() << " ";
    }
    std::cout << std::endl;
    return Number::Zero();
  }

 private:
   Console() :
    JSObject(
      OBJ_BOOL, u"Console", true, Bool::False(), false, false
    ) {
      AddFuncProperty(u"log", log, false, false, false);
    }
};

}  // namespace es

#endif  // ES_TYPES_HOST_CONSOLE