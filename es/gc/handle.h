#ifndef ES_GC_HANDLE_H
#define ES_GC_HANDLE_H

#include <vector>

#include <es/utils/macros.h>

namespace es {

class HeapObject;

class HandleScope {
 public:
  HandleScope() { HandleScope::Stack().emplace_back(this); }
  ~HandleScope() { HandleScope::Stack().pop_back(); }

  static HeapObject** Add(HeapObject* val) {
    std::cout << "Add---------------------------" << std::endl;
    HandleScope* current = HandleScope::Stack().back();
    size_t count_ = current->count_;
    HeapObject** ptr = current->pointers_ + count_;
    current->count_ += 1;
    *ptr = val;
    if (current->count_ >= kHandleScopeSize)
      assert(false);
    return ptr;
  }

  static std::vector<HandleScope*>& Stack() {
    static std::vector<HandleScope*> stack;
    return stack;
  }

  static bool In() {
    return HandleScope::Stack().size() > 0;
  }

  std::vector<HeapObject**> Pointers() {
    std::vector<HeapObject**> pointers(count_);
    for (size_t i = 0; i < count_; i++) {
      pointers[i] = pointers_ + i;
    }
    return pointers;
  }

 private:
  static constexpr size_t kHandleScopeSize = 1024 * 1024;

  HeapObject* pointers_[kHandleScopeSize];
  size_t count_ = 0;

  static HandleScope base;
};

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
    in_scope_ = HandleScope::In() && value != nullptr;
    if (in_scope_) {
      ptr_ = reinterpret_cast<T**>(HandleScope::Add(value));
    } else {
      val_ = value;
    }
  }

  explicit Handle() : in_scope_(false), val_(nullptr) {}

  template<typename S>
  Handle(Handle<S> base) {
    in_scope_ = base.in_scope();
    if (in_scope_) {
      ptr_ = reinterpret_cast<T**>(base.ptr());
    } else {
      val_ = static_cast<T*>(base.val());
    }
  }

  T* val() {
    if (in_scope_) {
      assert(ptr_ != nullptr);
      return *reinterpret_cast<T**>(ptr_);
    } else {
      return val_;
    }
  }

  T** ptr() {
    if (in_scope_) {
      return ptr_;
    } else {
      return &val_;
    }
  }

  bool in_scope() { return in_scope_; }

  bool IsNullptr() { return val() == nullptr; }

  std::string ToString() {
    if (IsNullptr())
      return "nullptr";
    return val()->ToString();
  }

 private:
  union {
    T** ptr_;
    T* val_;
  };
  bool in_scope_ = false;
};

}  // namespace es

#endif  // ES_GC_HANDLE_H