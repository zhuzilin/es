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

    Idx operator+(size_t offset) {
      size_t new_element_idx = (element_idx + offset) % kBlockSize;
      size_t new_block_idx = block_idx + (element_idx + offset) / kBlockSize;
      return {new_block_idx, new_element_idx};
    }
  };

  BlockStack() : stack_(1) {
    block_base_ = stack_[0].pointers_.get();
    cursor_ = block_base_;
    cursor_end_ = cursor_ + kBlockSize;
  }

  size_t num_elements() {
    size_t cur_offset = static_cast<size_t>(cursor_ - block_base_);
    return (size() - 1) * kBlockSize + cur_offset;
  }
  size_t last_block_offset() {
    return static_cast<size_t>(cursor_ - block_base_);
  }
  size_t size() { return stack_.size(); }
  Block& back() { return stack_.back(); }
  void pop_back() { stack_.pop_back(); }
  Block& operator [](size_t block_idx) { return stack_[block_idx]; }
  T* get(Idx idx) {
    assert(idx.block_idx < size() && idx.element_idx < kBlockSize);
    return stack_[idx.block_idx].pointers_.get() + idx.element_idx;
  }

  inline T* Add(T val) {
    if (likely(cursor_ < cursor_end_)) {
      *cursor_ = val;
      return cursor_++;
    }
    return AddSlow(val);
  }

  Idx GetNextPosition() {
    return {size() - 1, static_cast<size_t>(cursor_ - block_base_)};
  }

  void Rewind(Idx idx) {
    if (likely(stack_.size() == idx.block_idx + 1)) {
      // Common case: same block, just reset cursor
      cursor_ = block_base_ + idx.element_idx;
      return;
    }
    while (stack_.size() > idx.block_idx + 1) {
      stack_.pop_back();
    }
    block_base_ = stack_[idx.block_idx].pointers_.get();
    cursor_ = block_base_ + idx.element_idx;
    cursor_end_ = block_base_ + kBlockSize;
  }

 private:
  T* AddSlow(T val) {
    stack_.emplace_back(Block());
    Block& block = stack_.back();
    block.pointers_.get()[0] = val;
    block_base_ = block.pointers_.get();
    cursor_ = block_base_ + 1;
    cursor_end_ = block_base_ + kBlockSize;
    return block_base_;
  }

  std::vector<Block> stack_;
  T* block_base_;
  T* cursor_;
  T* cursor_end_;
};

}  // namespace es

#endif  // ES_UTILS_BLOCK_STACK
