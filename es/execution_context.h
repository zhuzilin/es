#ifndef ES_EXECUTION_CONTEXT_H
#define ES_EXECUTION_CONTEXT_H

#include <es/types/object.h>
#include <es/types/lexical_environment.h>

namespace es {

class ExecutionContext {
 public:

 private:
  LexicalEnvironment* variable_env_;
  LexicalEnvironment* lexical_env_;
  JSObject* this_binding_;
};

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H