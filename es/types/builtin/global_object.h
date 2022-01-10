#ifndef ES_TYPES_BUILTIN_GLOBAL_OBJECT
#define ES_TYPES_BUILTIN_GLOBAL_OBJECT

#include <es/types/object.h>

namespace es {

// 15.1 The Global Object
class GlobalObject : public JSObject {
 public:
  static GlobalObject* Instance() {
    static GlobalObject singleton;
    return &singleton;
  }

 private:
  GlobalObject() :
    JSObject(
      OBJ_GLOBAL,
      // 15.1 The values of the [[Prototype]] and [[Class]]
      // of the global object are implementation-dependent.
      Null::Instance(), u"Global",
      // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
      // otherwise we cannot define variable in global code, as global varaibles
      // are the property of global object.
      true,
      nullptr,
      false,
      false
    ) {}

  // 15.1.2.1 eval(X)
  JSValue* Eval(JSValue* val, Error* e) {
    if (val->type() != JSValue::JS_STRING)
      return val;
    // TODO(zhuzilin) finish eval
    e = Error::SyntaxError();
    return nullptr;
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_GLOBAL_OBJECT