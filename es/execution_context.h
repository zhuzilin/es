#ifndef ES_EXECUTION_CONTEXT_H
#define ES_EXECUTION_CONTEXT_H

#include <set>
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
      this_binding_(this_binding), strict_(strict), iteration_layers_(0) {}

  LexicalEnvironment* variable_env() { return variable_env_; }
  LexicalEnvironment* lexical_env() { return lexical_env_; }
  JSValue* this_binding() { return this_binding_; }
  bool strict() { return strict_; }

  void SetLexicalEnv(LexicalEnvironment* lexical_env) { lexical_env_ = lexical_env; }

  bool HasLabel(std::u16string label) {
    if (label == u"")
      return true;
    return label_set_.find(label) != label_set_.end();
  }

  void AddLabel(std::u16string label) {
    assert(!HasLabel(label));
    label_set_.insert(label);
  }

  void RemoveLabel(std::u16string label) {
    if (label == u"") return;
    assert(HasLabel(label));
    label_set_.erase(label);
  }

  void EnterIteration() { iteration_layers_++; }
  void ExitIteration() {
    if (iteration_layers_ != 0)
      iteration_layers_--;
  }
  bool InIteration() { return iteration_layers_ != 0; }

 private:
  LexicalEnvironment* variable_env_;
  LexicalEnvironment* lexical_env_;
  JSValue* this_binding_;
  bool strict_;
  std::set<std::u16string> label_set_;
  size_t iteration_layers_;
};

class ExecutionContextStack {
 public:
  static ExecutionContextStack* Global() {
    static ExecutionContextStack singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext* context) {
    context_stack_.push(context);
    if (context_stack_.size() == 1)
      global_env_ = context;
  }

  static ExecutionContext* TopContext() {
    return ExecutionContextStack::Global()->context_stack_.top();
  }

  static LexicalEnvironment* TopLexicalEnv() {
    return ExecutionContextStack::TopContext()->lexical_env();
  }

  void Pop() {
    ExecutionContext* top = context_stack_.top();
    context_stack_.pop();
    delete top;
  }

  ExecutionContext* global_env() { return global_env_; }

 private:
  ExecutionContextStack() = default;

  std::stack<ExecutionContext*> context_stack_;
  ExecutionContext* global_env_;
};

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H