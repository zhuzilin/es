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
    Handle<LexicalEnvironment> variable_env,
    Handle<LexicalEnvironment> lexical_env,
    Handle<JSValue> this_binding,
    bool strict
  ) : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding),
      strict_(strict), iteration_layers_(0), switch_layers_(0) {
  }

  Handle<LexicalEnvironment> variable_env() { return variable_env_; }
  Handle<LexicalEnvironment> lexical_env() { return lexical_env_; }
  Handle<JSValue> this_binding() { return this_binding_; }
  bool strict() { return strict_; }

  void SetLexicalEnv(Handle<LexicalEnvironment> lexical_env) {
    lexical_env_ = lexical_env;
  }

  bool HasLabel(std::u16string label) {
    if (label == u"")
      return true;
    return label_stack_.size() && label_stack_.top() == label;
  }

  void AddLabel(std::u16string label) {
    ASSERT(!HasLabel(label));
    label_stack_.push(label);
  }

  void RemoveLabel(std::u16string label) {
    if (label == u"") return;
    ASSERT(HasLabel(label));
    label_stack_.pop();
  }

  void EnterIteration() { iteration_layers_++; }
  void ExitIteration() {
    if (iteration_layers_ != 0)
      iteration_layers_--;
  }
  bool InIteration() { return iteration_layers_ != 0; }

  void EnterSwitch() { switch_layers_++; }
  void ExitSwitch() {
    if (switch_layers_ != 0)
      switch_layers_--;
  }
  bool InSwitch() { return switch_layers_ != 0; }

 private:
  Handle<LexicalEnvironment> variable_env_;
  Handle<LexicalEnvironment> lexical_env_;
  Handle<JSValue> this_binding_;

  bool strict_;
  std::stack<std::u16string> label_stack_;
  size_t iteration_layers_;
  size_t switch_layers_;
};

class Runtime {
 public:
  static Runtime* Global() {
    static Runtime singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext&& context) {
    context_stack_.emplace_back(std::move(context));
  }

  static ExecutionContext& TopContext() {
    return Runtime::Global()->context_stack_.back();
  }

  static Handle<LexicalEnvironment> TopLexicalEnv() {
    return Runtime::TopContext().lexical_env();
  }

  void PopContext() {
    context_stack_.pop_back();
  }

  static Handle<JSValue> TopValue() {
    return Runtime::Global()->value_stack_.back();
  }

  void AddValue(Handle<JSValue> val) {
    value_stack_.emplace_back(val);
  }

  void PopValue() {
    value_stack_.pop_back();
  }

  std::u16string AddSource(std::u16string&& source) {
    sources_.emplace_back(source);
    return sources_[sources_.size() - 1];
  }

  std::vector<HeapObject**> Pointers() {
    std::vector<HeapObject**> pointers(3 * context_stack_.size());
    for (size_t i = 0; i < context_stack_.size(); i++) {
      pointers[3 * i] = reinterpret_cast<HeapObject**>(context_stack_[i].variable_env().ptr());
      pointers[3 * i + 1] = reinterpret_cast<HeapObject**>(context_stack_[i].lexical_env().ptr());
      pointers[3 * i + 2] = reinterpret_cast<HeapObject**>(context_stack_[i].this_binding().ptr());
    }
    auto scope_pointers = HandleScope::AllPointers();
    pointers.insert(pointers.end(), scope_pointers.begin(), scope_pointers.end());
    return pointers;
  }

 private:
  Runtime() {
    value_stack_.emplace_back(Null::Instance());
  }

  std::vector<ExecutionContext> context_stack_;
  // This is to make sure builtin function like `array.push()`
  // can visit `array`.
  std::vector<Handle<JSValue>> value_stack_;
  std::vector<std::u16string> sources_;
};

class ValueGuard {
 public:
  ValueGuard() : count_(0) {}
  ~ValueGuard() {
    while (count_ > 0) {
      Runtime::Global()->PopValue();
      count_--;
    }
  }

  void AddValue(Handle<JSValue> val) {
    Runtime::Global()->AddValue(val);
    count_++;
  }

 private:
  size_t count_;
};

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H