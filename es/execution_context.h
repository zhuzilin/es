#ifndef ES_EXECUTION_CONTEXT_H
#define ES_EXECUTION_CONTEXT_H

#include <stack>

#include <es/types/object.h>
#include <es/types/lexical_environment.h>
#include <es/types/builtin/global_object.h>

namespace es {

class ExecutionContext;
class ExecutionContextStack {
 public:
  static ExecutionContextStack* Global() {
    static ExecutionContextStack singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext* context) {
    stack_.push(context);
  }

  ExecutionContext* Top() {
    return stack_.top();
  }

 private:
  ExecutionContextStack() = default;

  std::stack<ExecutionContext*> stack_;
};

class ExecutionContext {
 public: 
  ExecutionContext(
    LexicalEnvironment* variable_env,
    LexicalEnvironment* lexical_env,
    JSObject* this_binding)
    : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding) {}

  LexicalEnvironment* lexical_env() { return lexical_env_; }

 private:
  LexicalEnvironment* variable_env_;
  LexicalEnvironment* lexical_env_;
  JSObject* this_binding_;
};

// 10.4.1
void EnterGlobalCode() {
  LexicalEnvironment* global_env = new LexicalEnvironment(
    Null::Instance(), new ObjectEnvironmentRecord(GlobalObject::Instance()));
  // 10.4.1.1
  ExecutionContext* context = new ExecutionContext(
    global_env, global_env, GlobalObject::Instance());
  ExecutionContextStack::Global()->AddContext(context);
}

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H