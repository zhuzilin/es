#ifndef ES_UTILS_BLOCK_STACK
#define ES_UTILS_BLOCK_STACK

#include <stdlib.h>

namespace es {

template<typename T, size_t N>
class BlockStack {
 public:
  static constexpr size_t kBlockSize = N;

  struct Block {
    Block() :
      pointers_(new T[kBlockSize]), offset_(0) {}

    std::unique_ptr<T[]> pointers_;
    size_t offset_;
  };
  struct Idx {
    size_t block_idx;
    size_t element_idx;
  };

  BlockStack() : stack_(1) {}

  size_t num_elements() { return (size() - 1) * kBlockSize + back().offset_; }
  size_t size() { return stack_.size(); }
  Block& back() { return stack_.back(); }
  void pop_back() { stack_.pop_back(); }
  Block& operator [](size_t block_idx) { return stack_[block_idx]; }
  T* get(Idx idx) {
    ASSERT(idx.block_idx < size() && idx.element_idx < kBlockSize);
    return stack_[idx.block_idx].pointers_.get() + idx.element_idx;
  }

  T* Add(T val) {
    if (stack_.back().offset_ == kBlockSize) {
      stack_.emplace_back(Block());
    }
    Block& block = stack_.back();
    size_t offset = block.offset_;
    block.pointers_.get()[offset] = val;
    block.offset_++;
    return block.pointers_.get() + offset;
  }

  Idx GetNextPosition() {
    return {size() - 1, back().offset_};
  }

  void Rewind(Idx idx) {
    while (stack_.size() > idx.block_idx + 1) {
      stack_.pop_back();
    }
    stack_[idx.block_idx].offset_ = idx.element_idx;
  }

 private:
  std::vector<Block> stack_;
};

}  // namespace es

#endif  // ES_UTILS_BLOCK_STACK
