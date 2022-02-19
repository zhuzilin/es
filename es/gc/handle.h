#ifndef ES_GC_HANDLE_H
#define ES_GC_HANDLE_H

#include <vector>

#include <es/utils/macros.h>
#include <es/gc/header.h>

namespace es {

class HeapObject;

static constexpr size_t kHandleBlockSize = 10 * 1024;

struct HandleBlock {
  HandleBlock() {
    pointers = new HeapObject*[kHandleBlockSize];
    next = nullptr;
  }

  ~HandleBlock() {
    delete[] pointers;
    if (next != nullptr)
      delete next;
  }

  HeapObject** pointers;
  HandleBlock* next;
};

class HandleScope {
 public:
  HandleScope() {
    if (HandleScope::Stack().size() == 0) {
      start_block_ = new HandleBlock();
      current_ = start_block_->pointers;
      limit_ = start_block_->pointers + kHandleBlockSize;
    } else {
      HandleScope* last_scope = HandleScope::Stack().back();
      start_block_ = last_scope->block_;
      current_ = last_scope->current_;
      limit_ = last_scope->limit_;
    }
    block_ = start_block_;
    HandleScope::Stack().emplace_back(this);
  }
  ~HandleScope() {
    // The base scope won't be deleted, so we don't need to
    // consider the case where start_block_ was created for this HandleScope.
    if (start_block_->next != nullptr && start_block_->next->next != nullptr) {
      delete start_block_->next->next;
      start_block_->next->next = nullptr;
    }
    HandleScope::Stack().pop_back();
  }

  static HeapObject** Add(HeapObject* val) {
    if ((Flag(val) & GCFlag::CONST)) {
      if (constant_pointers_count_ == kNumConstantHandle) {
        throw std::runtime_error("too much constant handles");
      }
      for (size_t i = 0; i < constant_pointers_count_; i++) {
        if (constant_pointers_[i] == val) {
          return constant_pointers_ + i;
        }
      }
      HeapObject** ptr = constant_pointers_ + constant_pointers_count_;
      *ptr = val;
      constant_pointers_count_++;
      return ptr;
    } else if ((Flag(val) & GCFlag::SINGLE)) {
      if (singleton_pointers_count_ == kNumSingletonHandle) {
        throw std::runtime_error("too much singleton handles");
      }
      for (size_t i = 0; i < singleton_pointers_count_; i++) {
        if (singleton_pointers_[i] == val) {
          return singleton_pointers_ + i;
        }
      }
      HeapObject** ptr = singleton_pointers_ + singleton_pointers_count_;
      *ptr = val;
      singleton_pointers_count_++;
      return ptr;
    }

    HandleScope* scope = HandleScope::Stack().back();
    if (scope->current_ == scope->limit_) {
      HandleBlock* block = scope->block_->next;
      if (block == nullptr) {
        block = new HandleBlock();
        scope->block_->next = block;
      }
      scope->block_ = block;
      scope->current_ = block->pointers;
      scope->limit_ = block->pointers + kHandleBlockSize;
    }
    HeapObject** ptr = scope->current_;
    scope->current_++;
    *ptr = val;
#ifdef TEST
    scope->helper_count_++;
#endif
    
    return ptr;
  }

  static std::vector<HandleScope*>& Stack() {
    static std::vector<HandleScope*> stack;
    return stack;
  }

  static std::vector<HeapObject**> AllPointers() {
    std::vector<HeapObject**> pointers(singleton_pointers_count_);
    for (size_t i = 0; i < singleton_pointers_count_; i++) {
      pointers[i] = singleton_pointers_ + i;
    }
    HandleBlock* block = base.start_block_;
    // The last block may not be needed
    HandleBlock* end_ptr = Stack().back()->block_->next;
    while (block != end_ptr) {
      HeapObject** end = block->pointers + kHandleBlockSize;
      if (block->next == end_ptr) {
        end = Stack().back()->current_;
      }
      for (HeapObject** i = block->pointers; i != end; i++) {
        pointers.emplace_back(i);
      }
      block = block->next;
    }
#ifdef TEST
    size_t helper_sum = singleton_pointers_count_;
    for (auto s : Stack()) {
      helper_sum += s->helper_count_;
    }
    ASSERT(helper_sum == pointers.size());
#endif
    return pointers;
  }

 private:
  HandleBlock* start_block_;
  HandleBlock* block_;
  HeapObject** current_;
  HeapObject** limit_;
#ifdef TEST
  size_t helper_count_ = 0;
#endif

  static constexpr size_t kNumSingletonHandle = 32;
  static HeapObject* singleton_pointers_[kNumSingletonHandle];
  static size_t singleton_pointers_count_;

  static constexpr size_t kNumConstantHandle = 32;
  static HeapObject* constant_pointers_[kNumConstantHandle];
  static size_t constant_pointers_count_;

  static HandleScope base;
};

HeapObject* HandleScope::singleton_pointers_[kNumSingletonHandle];
size_t HandleScope::singleton_pointers_count_ = 0;

HeapObject* HandleScope::constant_pointers_[kNumConstantHandle];
size_t HandleScope::constant_pointers_count_ = 0;

HandleScope HandleScope::base;

// Handle is used to solve the following situation:
// ```
//   Value* a = New();
//   Value* b = New();  // triggered GC
//   Value* c = New(a, b);  // a need to point to new address
// ```
// By changing to
// ```
//   HandleScope scope; 
//   Handle<Value> a = New();
//   Handle<Value> b = New();  // triggered GC
//   Handle<Value> c = New(a, b);  // a need to point to new address
// ```
template<typename T>
class Handle {
 public:
  explicit Handle(T* value) {
    if (value != nullptr) {
#ifdef GC_DEBUG
      if (unlikely(log::Debugger::On())) {
        Handle<T> tmp;
        std::cout << "Add value " << typeid(tmp).name() << " flag: " << int(Flag(value)) << "\n";
      }
#endif
      ptr_ = reinterpret_cast<T**>(HandleScope::Add(value));
    } else {
      ptr_ = nullptr;
    }
  }

  explicit Handle() : ptr_(nullptr) {}

  template<typename S>
  Handle(Handle<S> base) {
#ifdef GC_DEBUG
    // Make sure the type conversion is correct.
    S* a = nullptr;
    T* b = static_cast<T*>(a);
#endif
    ptr_ = reinterpret_cast<T**>(base.ptr());
  }

  T* val() {
    if (ptr_ == nullptr)
      return nullptr;
    return *reinterpret_cast<T**>(ptr_);
  }

  T** ptr() {
    return ptr_;
  }

  bool IsNullptr() { return ptr() == nullptr; }

  std::string ToString();

 private:
  T** ptr_;
};

}  // namespace es

#endif  // ES_GC_HANDLE_H