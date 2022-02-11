#ifndef ES_GC_MARK_AND_SWEEP_COLLECTION_H
#define ES_GC_MARK_AND_SWEEP_COLLECTION_H

#include <string.h>

#include <stack>

#include <es/gc/base_collection.h>

namespace es {

class MarkAndSweepCollection : public GC {
 public:
  MarkAndSweepCollection(size_t size) {
    heap_start_ = static_cast<char*>(malloc(size));
    memset(heap_start_, 0, size);
    heap_end_ = heap_start_ + size;

    free_list_ = new Cell(heap_start_, size);
    first_obj_ = nullptr;
  }

 private:
  struct Cell {
    Cell(void* addr, size_t size) :
      addr(addr), size(size), prev_obj(nullptr), prev(nullptr), next(nullptr) {
      memset(addr, 0, size);
    }

    void* addr;
    size_t size;
    void* prev_obj;
    Cell* prev;
    Cell* next;
  };

  void* Allocate(size_t size, flag_t flag) override {
    // First-fit allocation
    Cell* cell = free_list_;
    while(cell != nullptr && cell->size < size) {
      cell = cell->next;
    }
    if (cell == nullptr)
      return nullptr;
    void* ptr = cell->addr;
    void* prev_obj = cell->prev_obj;
    if (cell->size < size + kMinCellSize) {
      if (cell->prev != nullptr) {
        cell->prev->next = cell->next;
      }
      if (cell->next != nullptr) {
        cell->next->prev = cell->prev;
      }
      if (cell == free_list_) {
        free_list_ = cell->next;
      }
      cell->prev = nullptr;
      cell->next = nullptr;
      delete cell;
    } else {
      cell->size -= size;
      cell->addr = static_cast<char*>(ptr) + size;
      assert(InHeap(cell->addr));
      cell->prev_obj = ptr;
    }
    // Set header
    Header* header = static_cast<Header*>(ptr);
    header->size = size;
    header->flag = flag;
    header->next_obj = nullptr;
    if (prev_obj != nullptr) {
      Header* prev_header = reinterpret_cast<Header*>(prev_obj);
      header->next_obj = prev_header->next_obj;
      prev_header->next_obj = ptr;
    } else {
      // When there is no prev_obj, the current obj is the first obj.
      header->next_obj = first_obj_;
      first_obj_ = ptr;
    }
    return ptr;
  }

  void Collect() override {
#ifdef GC_DEBUG
    std::cout << "enter MarkAndSweepCollection::Collect " << FreeSpace() / 1024U / 1024 << "\n";
#endif
    ClearFreeList();
    MarkFromRoot();
    Sweep();
#ifdef GC_DEBUG
    std::cout << "exit MarkAndSweepCollection::Collect " << FreeSpace() / 1024U / 1024 << "\n";
#endif
  }

  void MarkFromRoot() {
    std::stack<HeapObject*> worklist;
    auto root_pointers = Runtime::Global()->Pointers();
    assert(root_pointers.size() > 0);
    for (HeapObject** fld : root_pointers) {
      HeapObject* ref = *fld;
      if (ref != nullptr && !IsMarked(ref) && !(Flag(ref) & GCFlag::CONST)) {
        SetMarked(ref);
        worklist.push(ref);
        Mark(worklist);
      }
    }
  }

  void Mark(std::stack<HeapObject*>& worklist) {
    while (!worklist.empty()) {
      HeapObject* ref = worklist.top();
      worklist.pop();
      for (HeapObject** fld : HeapObject::Pointers(ref)) {
        HeapObject* child = *fld;
        if (child != nullptr && !IsMarked(child) && !(Flag(child) & GCFlag::CONST)) {
          SetMarked(child);
          worklist.push(child);
        }
      }
    }
  }

  void Sweep() {
    Cell* last_cell = nullptr;
    void* obj = first_obj_;
    while (obj != nullptr) {
      Header* header = static_cast<Header*>(obj);
      if (header->flag & GCFlag::MARK) {
        // For implementation simplicity, this may adds a zero cell.
        last_cell = new Cell(heap_start_, static_cast<char*>(obj) - heap_start_);
        break;
      }
      obj = header->next_obj;
    }
    if (obj == nullptr) {
      // All objects are not marked.
      free_list_ = new Cell(heap_start_, heap_end_ - heap_start_);
      first_obj_ = nullptr;
      return;
    }
    assert(last_cell != nullptr);
    free_list_ = last_cell;
    first_obj_ = obj;
    while (obj != nullptr) {
      // Unset Marked for obj
      Header* header = static_cast<Header*>(obj);
      assert(header->flag & GCFlag::MARK);
      header->flag = ~(~(header->flag) | GCFlag::MARK);
      // Find the next marked obj
      void* next_obj = header->next_obj;
      while (next_obj != nullptr) {
        Header* next_header = static_cast<Header*>(next_obj);
        if (next_header->flag & GCFlag::MARK) {
          break;
        }
        next_obj = next_header->next_obj;
      }
      char* cell_start = static_cast<char*>(obj) + header->size;
      char* cell_end = (next_obj == nullptr) ? heap_end_ : static_cast<char*>(next_obj);
      if (cell_start + kMinCellSize <= cell_end) {
        assert(InHeap(cell_start));
        Cell* cell = new Cell(cell_start, cell_end - cell_start);
        // Insert new cell to free list
        assert(free_list_ != nullptr);
        last_cell->next = cell;
        cell->prev = last_cell;
        cell->prev_obj = obj;
        last_cell = cell;
      }
      header->next_obj = next_obj;
      obj = next_obj;
    }
    // The first cell may be small.
    if (free_list_->size < kMinCellSize) {
      Cell* cell = free_list_;
      free_list_ = free_list_->next;
      cell->next = nullptr;
      delete cell;
    }
  }

  bool IsMarked(void* ref) {
    return Flag(ref) & GCFlag::MARK;
  }

  void SetMarked(void* ref) {
    H(ref)->flag = Flag(ref) | GCFlag::MARK;
  }

  void UnsetMarked(void* ref) {
    H(ref)->flag = ~(~Flag(ref) | GCFlag::MARK);
  }

#ifdef GC_DEBUG
  size_t FreeSpace() {
    size_t size = 0;
    Cell* cell = free_list_;
    while (cell != nullptr) {
      size += cell->size;
      assert(InHeap(cell->addr));
      assert(static_cast<char*>(cell->addr) + cell->size <= heap_end_);
      cell = cell->next;
    }
    return size;
  }
#endif

  // Use an iterative way to free the free list, because the
  // recursive way in destructor may stackoverflow.
  void ClearFreeList() {
    Cell* cell = free_list_;
    while (cell != nullptr) {
      Cell* old_cell = cell;
      cell = cell->next;
      delete old_cell;
    }
    free_list_ = nullptr;
  }

  bool InHeap(void* ptr) {
    return heap_start_ <= ptr && ptr < heap_end_;
  }

  char* heap_start_;
  char* heap_end_;

  Cell* free_list_;
  void* first_obj_;

  static constexpr size_t kMinCellSize = 32;
};

}  // namespace es

#endif  // ES_GC_MARK_AND_SWEEP_COLLECTION_H