#ifndef ES_EXECUTION_CONTEXT_H
#define ES_EXECUTION_CONTEXT_H

#include <stack>

#include <es/types/object.h>
#include <es/types/lexical_environment.h>
#include <es/types/builtin/global_object.h>

namespace es {

class ExecutionContext {
 public: 
  ExecutionContext(
    LexicalEnvironment* variable_env,
    LexicalEnvironment* lexical_env,
    JSValue* this_binding)
    : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding) {}

  LexicalEnvironment* variable_env() { return variable_env_; }
  LexicalEnvironment* lexical_env() { return lexical_env_; }

 private:
  LexicalEnvironment* variable_env_;
  LexicalEnvironment* lexical_env_;
  JSValue* this_binding_;
};

class ExecutionContextStack {
 public:
  static ExecutionContextStack* Global() {
    static ExecutionContextStack singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext context) {
    stack_.push(context);
  }

  ExecutionContext Top() {
    return stack_.top();
  }

  ExecutionContext Pop() {
    ExecutionContext top = stack_.top();
    stack_.pop();
    return top;
  }

 private:
  ExecutionContextStack() = default;

  std::stack<ExecutionContext> stack_;
};

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H