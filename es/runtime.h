#ifndef ES_EXECUTION_CONTEXT_H
#define ES_EXECUTION_CONTEXT_H

#include <set>
#include <stack>

#include <es/types/object.h>
#include <es/types/lexical_environment.h>
#include <es/types/reference.h>
#include <es/types/builtin/global_object.h>
#include <es/utils/block_stack.h>

namespace es {

class ExecutionContext {
 public:
  struct StackReference {
    Handle<JSValue> base;
    Handle<String> name;
  };

  using ReferenceBlockStack = BlockStack<StackReference, 1024>;

  ExecutionContext(
    Handle<EnvironmentRecord> variable_env,
    Handle<EnvironmentRecord> lexical_env,
    Handle<JSValue> this_binding,
    bool strict
  ) : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding),
      strict_(strict), iteration_layers_(0), switch_layers_(0),
      start_idx_(ref_block_stack_.GetNextPosition()), num_references_(0) {
  }

  // could not do this in ~Execution
  // as the std::vector resize will trigger the destructor
  void Rewind() {
    ref_block_stack_.Rewind(start_idx_);
  }

  Handle<EnvironmentRecord> variable_env() { return variable_env_; }
  Handle<EnvironmentRecord> lexical_env() { return lexical_env_; }
  Handle<JSValue> this_binding() { return this_binding_; }
  bool strict() { return strict_; }

  void SetLexicalEnv(Handle<EnvironmentRecord> lexical_env) {
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

  Handle<Reference> AddReference(Handle<JSValue> base, Handle<String> name) {
    // Must create ref before add to block stack.
    Handle<Reference> ref = Reference::New(num_references_);
    ref_block_stack_.Add({base, name});
    num_references_++;
    return ref;
  }

  StackReference GetReference(size_t i) {
    ASSERT(i < num_references_);
    return *ref_block_stack_.get(start_idx_ + i);
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

  static ReferenceBlockStack& ref_block_stack() { return ref_block_stack_; }

 private:
  Handle<EnvironmentRecord> variable_env_;
  Handle<EnvironmentRecord> lexical_env_;
  Handle<JSValue> this_binding_;

  bool strict_;
  std::stack<std::u16string> label_stack_;
  size_t iteration_layers_;
  size_t switch_layers_;

  static ReferenceBlockStack ref_block_stack_;
  ReferenceBlockStack::Idx start_idx_;
  size_t num_references_;
};

ExecutionContext::ReferenceBlockStack ExecutionContext::ref_block_stack_;

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

  static Handle<EnvironmentRecord> TopLexicalEnv() {
    return Runtime::TopContext().lexical_env();
  }

  void PopContext() {
    ExecutionContext& context = context_stack_.back();
    context.Rewind();
    ASSERT(!context.lexical_env().IsNullptr());
    //context.lexical_env().val()->ReduceRefCount();
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
    auto& ref_block_stack = ExecutionContext::ref_block_stack();
    size_t num_context_pointers = context_stack_.size() * 3 + ref_block_stack.num_elements() * 2;
    std::vector<HeapObject**> pointers(num_context_pointers);
    for (size_t i = 0; i < context_stack_.size(); ++i) {
      pointers[3 * i] = reinterpret_cast<HeapObject**>(context_stack_[i].lexical_env().ptr());
      pointers[3 * i + 1] = reinterpret_cast<HeapObject**>(context_stack_[i].variable_env().ptr());
      pointers[3 * i + 2] = reinterpret_cast<HeapObject**>(context_stack_[i].this_binding().ptr());
    }
    size_t offset = 3 * context_stack_.size();
    for (size_t i = 0; i < ref_block_stack.size(); ++i) {
      size_t limit = i == ref_block_stack.size() - 1 ?
        ref_block_stack.back().offset_ :
        ExecutionContext::ReferenceBlockStack::kBlockSize;
      for (size_t j = 0; j < limit; ++j) {
        pointers[offset + 2 * j] = reinterpret_cast<HeapObject**>(ref_block_stack.get({i, j})->base.ptr());
        pointers[offset + 2 * j + 1] = reinterpret_cast<HeapObject**>(ref_block_stack.get({i, j})->name.ptr());
      }
      offset += 2 * ExecutionContext::ReferenceBlockStack::kBlockSize;
    }
    auto scope_pointers = HandleScope::AllPointers();
    pointers.insert(pointers.end(), scope_pointers.begin(), scope_pointers.end());
    auto extra_pointers = ExtracGC::Pointers();
    pointers.insert(pointers.end(), extra_pointers.begin(), extra_pointers.end());
#ifdef GC_DEBUG
    for (size_t i = 0; i < pointers.size(); ++i) {
      assert(pointers[i] != nullptr);
    }
#endif
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