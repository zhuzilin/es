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
    return label_stack_.size() && label_stack_.top() == label;
  }

  void AddLabel(std::u16string label) {
    assert(!HasLabel(label));
    label_stack_.push(label);
  }

  void RemoveLabel(std::u16string label) {
    if (label == u"") return;
    assert(HasLabel(label));
    label_stack_.pop();
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
  std::stack<std::u16string> label_stack_;
  size_t iteration_layers_;
};

class RuntimeContext {
 public:
  static RuntimeContext* Global() {
    static RuntimeContext singleton;
    return &singleton;
  }

  void AddContext(ExecutionContext* context) {
    context_stack_.push(context);
    if (context_stack_.size() == 1)
      global_env_ = context;
  }

  static ExecutionContext* TopContext() {
    return RuntimeContext::Global()->context_stack_.top();
  }

  static LexicalEnvironment* TopLexicalEnv() {
    return RuntimeContext::TopContext()->lexical_env();
  }

  void PopContext() {
    ExecutionContext* top = context_stack_.top();
    context_stack_.pop();
    delete top;
  }

  static JSValue* TopValue() {
    return RuntimeContext::Global()->value_stack_.top();
  }

  void AddValue(JSValue* val) {
    value_stack_.push(val);
  }

  void PopValue() {
    value_stack_.pop();
  }

  ExecutionContext* global_env() { return global_env_; }

 private:
  RuntimeContext() {
    value_stack_.push(Null::Instance());
  }

  std::stack<ExecutionContext*> context_stack_;
  ExecutionContext* global_env_;
  // This is to make sure builtin function like `array.push()`
  // can visit `array`.
  std::stack<JSValue*> value_stack_;
};

class ValueGuard {
 public:
  ValueGuard() : count_(0) {}
  ~ValueGuard() {
    while (count_ > 0) {
      RuntimeContext::Global()->PopValue();
      count_--;
    }
  }

  void AddValue(JSValue* val) {
    RuntimeContext::Global()->AddValue(val);
    count_++;
  }

 private:
  size_t count_;
};

JSValue* JSObject::DefaultValue(Error* e, std::u16string hint) {
  std::u16string first, second;
  if (hint == u"String" || hint == u"" && obj_type() == OBJ_DATE) {
    first = u"toString";
    second = u"valueOf";
  } else if (hint == u"Number" || hint == u"" && obj_type() != OBJ_DATE) {
    first = u"valueOf";
    second = u"toString";
  } else {
    assert(false);
  }

  ValueGuard guard;
  guard.AddValue(this);

  JSValue* to_string = Get(e, first);
  if (!e->IsOk()) return nullptr;
  if (to_string->IsCallable()) {
    JSObject* to_string_obj = static_cast<JSObject*>(to_string);
    JSValue* str = to_string_obj->Call(e, this);
    if (!e->IsOk()) return nullptr;
    if (str->IsPrimitive()) {
      return str;
    }
  }
  JSValue* value_of = Get(e, second);
  if (!e->IsOk()) return nullptr;
  if (value_of->IsCallable()) {
    JSObject* value_of_obj = static_cast<JSObject*>(value_of);
    JSValue* val = value_of_obj->Call(e, this);
    if (!e->IsOk()) return nullptr;
    if (val->IsPrimitive()) {
      return val;
    }
  }
  *e = *Error::TypeError(u"failed to get [[DefaultValue]]");
  return nullptr;
}

}  // namespace es

#endif  // ES_EXECUTION_CONTEXT_H