#ifndef ES_GC_HANDLE_H
#define ES_GC_HANDLE_H

#include <vector>

#include <es/utils/macros.h>
#include <es/utils/block_stack.h>
#include <es/gc/header.h>

namespace es {

class HeapObject;

constexpr size_t kNumSingletonHandle = 32;
constexpr size_t kNumConstantHandle = 10 * 1024 * 1024;  // 10M

class HandleScope {
 public:
  using HandleBlockStack = BlockStack<HeapObject*, 10 * 1024>;

  HandleScope() {
    start_idx_ = block_stack_.GetNextPosition();
  }

  ~HandleScope() {
    block_stack_.Rewind(start_idx_);
  }

  static HeapObject** Add(HeapObject* val) {
    if (reinterpret_cast<uint64_t>(val) & STACK_MASK)
      goto normal;
#ifdef PARSER_ONLY
    assert(Flag(val) & GCFlag::CONST);
#endif
    if ((Flag(val) & GCFlag::CONST)) {
      if (constant_pointers_map_.size() == kNumConstantHandle) {
        throw std::runtime_error("too much constant handles");
      }
      auto iter = constant_pointers_map_.find(val);
      if (iter != constant_pointers_map_.end()) {
        size_t offset = iter->second;
        return constant_pointers_ + offset;
      }
      size_t offset = constant_pointers_map_.size();
      HeapObject** ptr = constant_pointers_ + offset;
      *ptr = val;
      constant_pointers_map_[val] = offset;
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
normal:
    return block_stack_.Add(val);
  }

  static std::vector<HeapObject**> AllPointers() {
    size_t num_pointers = singleton_pointers_count_;
    if (likely(block_stack_.size() > 0)) {
      num_pointers += block_stack_.num_elements();
    }
    std::vector<HeapObject**> pointers(num_pointers);
    for (size_t i = 0; i < singleton_pointers_count_; i++) {
      pointers[i] = singleton_pointers_ + i;
    }
    size_t offset = singleton_pointers_count_;
    for (size_t i = 0; i < block_stack_.size(); i++) {
      size_t limit = i == block_stack_.size() - 1 ? block_stack_.back().offset_ : HandleBlockStack::kBlockSize;
      for (size_t j = 0; j < limit; j++) {
        pointers[offset + j] = block_stack_.get({i, j});
      }
      offset += block_stack_.kBlockSize;
    }
    return pointers;
  }

 private:
  HandleBlockStack::Idx start_idx_;

  static HeapObject* singleton_pointers_[kNumSingletonHandle];
  static size_t singleton_pointers_count_;

  static HeapObject* constant_pointers_[kNumConstantHandle];
  static std::unordered_map<HeapObject*, uint32_t> constant_pointers_map_;

  static HandleBlockStack block_stack_;
};

HeapObject* HandleScope::singleton_pointers_[kNumSingletonHandle];
size_t HandleScope::singleton_pointers_count_ = 0;
HandleScope::HandleBlockStack HandleScope::block_stack_;

HeapObject* HandleScope::constant_pointers_[kNumConstantHandle];
std::unordered_map<HeapObject*, uint32_t> HandleScope::constant_pointers_map_;

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
      }
#endif
      ptr_ = reinterpret_cast<T**>(HandleScope::Add(reinterpret_cast<HeapObject*>(value)));
    } else {
      ptr_ = nullptr;
    }
  }

  explicit Handle() : ptr_(nullptr) {}

  template<typename S>
  Handle(Handle<S> base) {
#ifdef GC_DEBUG
    // Make sure the type conversion is correct.
    // S* a = nullptr;
    // T* b = static_cast<T*>(a);
#endif
    ptr_ = reinterpret_cast<T**>(base.ptr());
  }

  T* val() const {
    if (ptr_ == nullptr)
      return nullptr;
    return *reinterpret_cast<T**>(ptr_);
  }

  T** ptr() const {
    return ptr_;
  }

  bool IsNullptr() const { return ptr() == nullptr; }

  std::string ToString() const;

 private:
  T** ptr_;
};

}  // namespace es

#endif  // ES_GC_HANDLE_H