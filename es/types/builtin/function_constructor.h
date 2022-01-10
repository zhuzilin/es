#ifndef ES_TYPES_BUILTIN_FUNCTION_CONSTRUCTOR
#define ES_TYPES_BUILTIN_FUNCTION_CONSTRUCTOR

#include <es/types/builtin/function_object.h>

namespace es {

// "Function"
class FunctionConstructor : public Function {
 public:

 private:
  FunctionConstructor() :
    Function(new FunctionProto::Instance(), true) {
    auto prototype = new NamedDataProperty(new FunctionProto::Instance(), false, false, false);
    named_properties_.emplace(u"prototype", prototype);
    auto length = new NamedDataProperty(new Number(1), false, false, false);
    named_properties_.emplace(u"length", length);
  }
};

}

#endif  // ES_TYPES_BUILTIN_FUNCTION_CONSTRUCTOR