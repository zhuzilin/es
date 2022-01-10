#ifndef ES_TYPES_BUILTIN_FUNCTION_CONSTRUCTOR
#define ES_TYPES_BUILTIN_FUNCTION_CONSTRUCTOR

#include <es/types/builtin/function_object.h>

namespace es {

// "Function"
class FunctionConstructor : public Function {
 public:

 private:
  FunctionConstructor() :
    Function(new FunctionProto::Instance(), true) {}
};

}

#endif  // ES_TYPES_BUILTIN_FUNCTION_CONSTRUCTOR