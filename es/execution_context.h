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
    JSValue* this_binding,
    bool strict
  ) : variable_env_(variable_env), lexical_env_(lexical_env),
      this_binding_(this_binding), strict_(strict) {}

  LexicalEnvironment* variable_env() { return variable_env_; }
  LexicalEnvironment* lexical_env() { return lexical_env_; }
  JSValue* this_binding() { return this_binding_; }
  bool strict() { return strict_; }

 private:
  LexicalEnvironment* variable_env_;
  LexicalEnvironment* lexical_env_;
  JSValue* this_binding_;
  bool strict_;
};

class ExecutionContextStack {
 public:
  static ExecutionContextStack* Global() {
    static ExecutionContextStack singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext context) {
    stack_.push(context);
    if (stack_.size() == 1)
      global_env_ = &context;
  }

  ExecutionContext Top() {
    return stack_.top();
  }

  LexicalEnvironment* TopLexicalEnv() {
    return Top().lexical_env();
  }

  ExecutionContext Pop() {
    ExecutionContext top = stack_.top();
    stack_.pop();
    return top;
  }

  ExecutionContext* global_env() { return global_env_; }

 private:
  ExecutionContextStack() = default;

  std::stack<ExecutionContext> stack_;
  ExecutionContext* global_env_;
};

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H