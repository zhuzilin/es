#ifndef ES_TYPES_BUILTIN_GLOBAL_OBJECT
#define ES_TYPES_BUILTIN_GLOBAL_OBJECT

#include <es/types/object.h>

namespace es {

class GlobalObject : public JSObject {
  GlobalObject() :
    JSObject(
      OBJ_GLOBAL,
      // 15.1 The values of the [[Prototype]] and [[Class]]
      // of the global object are implementation-dependent.
      Null::Instance(), u"Global",
      // NOTE(zhuzilin) I'm not sure [[Extensible]] of global variable...
      false,
      nullptr, false, false
    ) {
    // Value Properties
    auto nan = new NamedDataProperty(new Number(Number::NaN), false, false, false);
    named_properties_.emplace(u"NaN", nan);
    auto infinity = new NamedDataProperty(new Number(0, 1), false, false, false);
    named_properties_.emplace(u"Infinity", infinity);
    auto undefined = new NamedDataProperty(Undefined::Instance(), false, false, false);
    named_properties_.emplace(u"undefined", undefined);
    // Function Properties

  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_GLOBAL_OBJECT