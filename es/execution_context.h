#ifndef ES_EXECUTION_CONTEXT_H
#define ES_EXECUTION_CONTEXT_H

#include <stack>

#include <es/types/object.h>
#include <es/types/lexical_environment.h>

namespace es {

class ExecutionContext;
class ExecutionContextStack {
 public:
  static ExecutionContextStack* Instance() {
    static ExecutionContextStack singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext* context) {
    context_stack_.push(context);
  }

 private:
  ExecutionContextStack() = default;

  std::stack<ExecutionContext*> context_stack_;
};

class ExecutionContext {
 public: 
  ExecutionContext(
    LexicalEnvironment* variable_env,
    LexicalEnvironment* lexical_env,
    JSObject* this_binding)
    : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding) {
    ExecutionContextStack::Instance()->AddContext(this);
  }

 private:
  LexicalEnvironment* variable_env_;
  LexicalEnvironment* lexical_env_;
  JSObject* this_binding_;
};

void EnterGlobalCode() {
  
}

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H