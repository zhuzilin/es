#ifndef ES_GC_HANDLE_H
#define ES_GC_HANDLE_H

#include <vector>

#include <es/utils/macros.h>
#include <es/utils/block_stack.h>
#include <es/gc/header.h>

namespace es {

class HeapObject;

constexpr size_t kNumSingletonHandle = 32;
constexpr size_t kNumConstantHandle = 256 * 1024;

// Open-addressing hash table for constant handle dedup.
// Stores index into constant_pointers_ array.
struct ConstantHandleMap {
  struct Entry {
    HeapObject* key;
    uint32_t value;
  };

  Entry* entries;
  uint32_t capacity;
  uint32_t mask;

  void Init(uint32_t initial_capacity) {
    capacity = initial_capacity;
    mask = capacity - 1;
    entries = new Entry[capacity];
    memset(entries, 0, capacity * sizeof(Entry));
  }

  // Returns pointer to value slot if found, nullptr otherwise.
  uint32_t* Find(HeapObject* key) {
    uint32_t h = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(key) >> 4);
    h ^= h >> 16;
    uint32_t idx = h & mask;
    while (true) {
      Entry& e = entries[idx];
      if (e.key == nullptr) return nullptr;
      if (e.key == key) return &e.value;
      idx = (idx + 1) & mask;
    }
  }

  void Insert(HeapObject* key, uint32_t value) {
    uint32_t h = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(key) >> 4);
    h ^= h >> 16;
    uint32_t idx = h & mask;
    while (true) {
      Entry& e = entries[idx];
      if (e.key == nullptr) {
        e.key = key;
        e.value = value;
        return;
      }
      idx = (idx + 1) & mask;
    }
  }

  void Grow(uint32_t new_capacity, HeapObject** constant_pointers, uint32_t count) {
    delete[] entries;
    capacity = new_capacity;
    mask = capacity - 1;
    entries = new Entry[capacity];
    memset(entries, 0, capacity * sizeof(Entry));
    for (uint32_t i = 0; i < count; i++) {
      Insert(constant_pointers[i], i);
    }
  }
};

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
    uint64_t raw = reinterpret_cast<uint64_t>(val);
    if (raw & STACK_MASK) {
      // For fixed stack-type singletons (Undefined, Null, Bool true/false),
      // use dedicated static slots to avoid block_stack_ overhead.
      if (raw < kNumStackSlots) {
        stack_slots_[raw] = val;
        return &stack_slots_[raw];
      }
      return block_stack_.Add(val);
    }
#ifdef PARSER_ONLY
    assert(Flag(val) & GCFlag::CONST);
#endif
    // Fast path: most heap objects are neither CONST nor SINGLE
    flag_t flag = Flag(val);
    if (likely(!(flag & (GCFlag::CONST | GCFlag::SINGLE)))) {
      return block_stack_.Add(val);
    }
    if ((flag & GCFlag::CONST)) {
      uint32_t* existing = constant_map_.Find(val);
      if (existing) {
        return constant_pointers_ + *existing;
      }
      if (constant_pointers_count_ == kNumConstantHandle) {
        throw std::runtime_error("too much constant handles");
      }
      // Check if hash table needs growth (load factor > 0.7)
      if (constant_pointers_count_ * 10 >= constant_map_.capacity * 7) {
        constant_map_.Grow(constant_map_.capacity * 2, constant_pointers_, constant_pointers_count_);
      }
      uint32_t offset = constant_pointers_count_++;
      constant_pointers_[offset] = val;
      constant_map_.Insert(val, offset);
      return constant_pointers_ + offset;
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
      size_t limit = i == block_stack_.size() - 1 ? block_stack_.last_block_offset() : HandleBlockStack::kBlockSize;
      for (size_t j = 0; j < limit; j++) {
        pointers[offset + j] = block_stack_.get({i, j});
      }
      offset += block_stack_.kBlockSize;
    }
    return pointers;
  }

 private:
  HandleBlockStack::Idx start_idx_;

  static constexpr size_t kNumStackSlots = 16;

  static HeapObject* singleton_pointers_[kNumSingletonHandle];
  static size_t singleton_pointers_count_;

  static HeapObject* constant_pointers_[kNumConstantHandle];
  static uint32_t constant_pointers_count_;
  static ConstantHandleMap constant_map_;

  static HandleBlockStack block_stack_;
  static HeapObject* stack_slots_[kNumStackSlots];
};

HeapObject* HandleScope::singleton_pointers_[kNumSingletonHandle];
size_t HandleScope::singleton_pointers_count_ = 0;
HandleScope::HandleBlockStack HandleScope::block_stack_;
HeapObject* HandleScope::stack_slots_[HandleScope::kNumStackSlots] = {};

HeapObject* HandleScope::constant_pointers_[kNumConstantHandle];
uint32_t HandleScope::constant_pointers_count_ = 0;

namespace {
  ConstantHandleMap MakeInitialConstantMap() {
    ConstantHandleMap m;
    m.Init(8192);
    return m;
  }
}
ConstantHandleMap HandleScope::constant_map_ = MakeInitialConstantMap();

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