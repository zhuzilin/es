#ifndef ES_GC_HANDLE_H
#define ES_GC_HANDLE_H

#include <vector>

#include <es/utils/macros.h>
#include <es/gc/header.h>

namespace es {

class HeapObject;

constexpr size_t kHandleBlockSize = 10 * 1024;
constexpr size_t kNumSingletonHandle = 32;
constexpr size_t kNumConstantHandle = 32;

struct HandleBlock {
  HandleBlock() :
    pointers_(new HeapObject*[kHandleBlockSize]), offset_(0) {}

  std::unique_ptr<HeapObject*[]> pointers_;
  size_t offset_;
};

class HandleScope {
 public:
  HandleScope() {
    if (block_stack_.size() == 0) {
      block_stack_.emplace_back(HandleBlock());
    }
    start_block_idx_ = block_stack_.size() - 1;
    start_block_offset_ = block_stack_.back().offset_;
  }

  ~HandleScope() {
    while (block_stack_.size() > start_block_idx_ + 1) {
      block_stack_.pop_back();
    }
    block_stack_[start_block_idx_].offset_ = start_block_offset_;
  }

  static HeapObject** Add(HeapObject* val) {
    if (reinterpret_cast<uint64_t>(val) & STACK_MASK)
      goto normal;
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
normal:
    if (block_stack_.back().offset_ == kHandleBlockSize) {
      block_stack_.emplace_back(HandleBlock());
    }
    HandleBlock& block = block_stack_.back();
    size_t offset = block.offset_;
    block.pointers_.get()[offset] = val;
    block.offset_++;
    return block.pointers_.get() + offset;
  }

  static std::vector<HeapObject**> AllPointers() {
    size_t num_pointers = singleton_pointers_count_;
    if (likely(block_stack_.size() > 0)) {
      num_pointers += (block_stack_.size() - 1) * kHandleBlockSize + block_stack_.back().offset_;
    }
    std::vector<HeapObject**> pointers(num_pointers);
    for (size_t i = 0; i < singleton_pointers_count_; i++) {
      pointers[i] = singleton_pointers_ + i;
    }
    size_t offset = singleton_pointers_count_;
    for (size_t i = 0; i < block_stack_.size() - 1; i++) {
      for (size_t j = 0; j < kHandleBlockSize; j++) {
        pointers[offset + j] = block_stack_[i].pointers_.get() + j;
      }
      offset += kHandleBlockSize;
    }
    for (size_t j = 0; j < block_stack_.back().offset_; j++) {
      pointers[offset + j] = block_stack_.back().pointers_.get() + j;
    }
    return pointers;
  }

 private:
  size_t start_block_idx_;
  size_t start_block_offset_;

  static HeapObject* singleton_pointers_[kNumSingletonHandle];
  static size_t singleton_pointers_count_;

  static HeapObject* constant_pointers_[kNumConstantHandle];
  static size_t constant_pointers_count_;

  static std::vector<HandleBlock> block_stack_;
};

HeapObject* HandleScope::singleton_pointers_[kNumSingletonHandle];
size_t HandleScope::singleton_pointers_count_ = 0;

HeapObject* HandleScope::constant_pointers_[kNumConstantHandle];
size_t HandleScope::constant_pointers_count_ = 0;

std::vector<HandleBlock> HandleScope::block_stack_;

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