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

  // 15.1.2.1 eval(X)
  static JSValue* eval(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  // 15.1.2.2 parseInt (string , radix)
  static JSValue* parseInt(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  // 15.1.2.3 parseFloat (string)
  static JSValue* parseFloat(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  // 15.1.2.4 isNaN (number)
  static JSValue* isNaN(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  // 15.1.2.5 isFinite (number)
  static JSValue* isFinite(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  GlobalObject() :
    JSObject(
      OBJ_GLOBAL,
      // 15.1 The values of the [[Prototype]] and [[Class]]
      // of the global object are implementation-dependent.
      u"Global",
      // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
      // otherwise we cannot define variable in global code, as global varaibles
      // are the property of global object.
      true,
      nullptr,
      false,
      false
    ) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_GLOBAL_OBJECT