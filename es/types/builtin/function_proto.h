#ifndef ES_TYPES_BUILTIN_FUNCTION_PROTO
#define ES_TYPES_BUILTIN_FUNCTION_PROTO

#include <es/types/builtin/function_object.h>

namespace es {

class FunctionProto : public Function {

};


// "Function"
class FunctionConstructor : public Function {
 public:

 private:
  FunctionConstructor() :
    Function(new FunctionProto::Instance(), true) {}
};

}

#endif  // ES_TYPES_BUILTIN_FUNCTION_PROTO