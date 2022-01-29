#ifndef ES_GC_HANDLE_H
#define ES_GC_HANDLE_H

#include <vector>

#include <es/utils/macros.h>
#include <es/gc/header.h>

namespace es {

class HeapObject;

class HandleScope {
 public:
  HandleScope() {
    std::cout << "enter HandleScope() " << this << std::endl;
    pointers_ = new HeapObject*[kHandleScopeSize];
    HandleScope::Stack().emplace_back(this);
  }
  ~HandleScope() { 
    std::cout << "enter ~HandleScope() " << this << std::endl;
    delete[] pointers_;
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
    HandleScope* current = HandleScope::Stack().back();
    size_t count_ = current->count_;
    HeapObject** ptr = current->pointers_ + count_;
    current->count_ += 1;
    *ptr = val;
    if (current->count_ >= kHandleScopeSize) {
      throw std::runtime_error("too much local handles");
    }
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

  static std::vector<HeapObject**> AllPointers() {
    std::vector<HeapObject**> pointers(singleton_pointers_count_);
    for (size_t i = 0; i < singleton_pointers_count_; i++) {
      pointers[i] = singleton_pointers_ + i;
    }
    for (HandleScope* scope : Stack()) {
      auto scope_pointers = scope->Pointers();
      pointers.insert(pointers.end(), scope_pointers.begin(), scope_pointers.end());
    }
    return pointers;
  }

 private:
  static constexpr size_t kHandleScopeSize = 1024 * 1024;

  HeapObject** pointers_;
  size_t count_ = 0;

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
    ptr_ = reinterpret_cast<T**>(HandleScope::Add(value));
  }

  explicit Handle() : ptr_(nullptr) {}

  template<typename S>
  Handle(Handle<S> base) {
#ifdef TEST
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

  std::string ToString() {
    if (IsNullptr())
      return "nullptr";
    return val()->ToString();
  }

 private:
  T** ptr_;
};

}  // namespace es

#endif  // ES_GC_HANDLE_H