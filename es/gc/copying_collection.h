#ifndef ES_GC_COPYING_COLLECTION
#define ES_GC_COPYING_COLLECTION

#include <stdlib.h>

#include <algorithm>
#include <map>

#include <es/gc/base_collection.h>
#include <es/utils/helper.h>

namespace es {

inline void MemCopy(void* dst, void* src, size_t size) {
  void** dst_p = reinterpret_cast<void**>(dst);
  void** src_p = reinterpret_cast<void**>(src);
  size_t count = size / kPtrSize;
  // This constant is taken from v8.
  if (count < 16) {
    do {
      count--;
      *dst_p++ = *src_p++;
    } while (count > 0);
  } else {
    memcpy(dst, src, size);
  }
}

struct CopyingCollection : public GC<CopyingCollection> {
  CopyingCollection(size_t size) {
    heap_start_ = static_cast<char*>(malloc(size));
    memset(heap_start_, 0, size);
    heap_end_ = heap_start_ + size;
    CreateSemispaces();
  }

  void CreateSemispaces() {
    tospace_ = heap_start_;
    extent_ = (heap_end_ - heap_start_) / 2;
    fromspace_ = heap_start_ + extent_;
    top_ = fromspace_;
    free_ = tospace_;
  }

  void* AllocateImpl(size_t size, flag_t flag) {
    char* result = free_;
    char* newfree = result + size;
    if (newfree > top_)
      return nullptr;
    free_ = newfree;
    // Set header
    Header* header = reinterpret_cast<Header*>(result);
    header->size = size;
    header->flag = flag;
    header->forward_address = nullptr;
    return result;
  }

  void CollectImpl() {
#ifdef GC_DEBUG
    std::cout << "enter CopyingCollection::Collect" << "\n";
#endif
#ifdef STATS
    Stats();
#endif
    Flip();
    Initialise(worklist_);
    auto root_pointers = Runtime::Global()->Pointers();
    ASSERT(root_pointers.size() > 0);
    for (HeapObject** fld : root_pointers) {
      Process(fld);
    }
    while (!IsEmpty(worklist_)) {
      void* ref = Remove(worklist_);
      Scan(ref);
    }
    memset(fromspace_, 0, extent_);
#ifdef GC_DEBUG
    std::cout << "exit CopyingCollection::Collect " << (free_ - tospace_) / 1024U / 1024U << "\n";
#endif
#ifdef STATS
    Stats();
#endif
  }

  void Flip() {
    std::swap(fromspace_, tospace_);
    top_ = tospace_ + extent_;
    free_ = tospace_;
    memset(tospace_, 0, extent_);
  }

  void Scan(void* ref) {
    HeapObject* heap_ref = static_cast<HeapObject*>(ref);
    ASSERT(heap_ref != nullptr);
    auto ref_pointers = HeapObject::Pointers(heap_ref);
    for (HeapObject** fld : ref_pointers) {
      Process(fld);
    }
  }

  void Process(HeapObject** fld) {
    if (*fld == nullptr || (Flag(*fld) & GCFlag::CONST))
      return;
    HeapObject* from_ref = *fld;
#ifdef GC_DEBUG
    ASSERT(from_ref != nullptr);
#endif
    if (InToSpace(from_ref)) {
      return;
    }
#ifdef GC_DEBUG
    ASSERT(InFromSpace(from_ref));
#endif
    *fld = static_cast<HeapObject*>(Forward(from_ref));
  }

  void* Forward(void* from_ref) {
    void* to_ref = ForwardAddress(from_ref);
    if (to_ref == nullptr) {
      to_ref = Copy(from_ref);
    }
#ifdef GC_DEBUG
    ASSERT(InToSpace(to_ref));
#endif
    return to_ref;
  }

  void* Copy(void* from_ref) {
    ASSERT(InFromSpace(from_ref));
    char* to_ref = free_ + sizeof(Header);
    size_t size = Size(from_ref);
    free_ += size;
#ifdef GC_DEBUG
    ASSERT(InToSpace(free_) || free_ == tospace_ + extent_);
    ASSERT(ForwardAddress(from_ref) == nullptr);
#endif
    MemCopy(H(to_ref), H(from_ref), size);
    SetForwardAddress(from_ref, to_ref);
#ifdef GC_DEBUG
    ASSERT(ForwardAddress(to_ref) == nullptr);
    ASSERT(InToSpace(ForwardAddress(from_ref)));
#endif
    Add(worklist_, to_ref);
    return to_ref;
  }

  bool InToSpace(void* ptr) {
    return tospace_ <= ptr && ptr < tospace_ + extent_;
  }

  bool InFromSpace(void* ptr) {
    return fromspace_ <= ptr && ptr < fromspace_ + extent_;
  }

  bool InHeap(void* ptr) {
    return heap_start_ <= ptr && ptr < heap_end_;
  }

  char* heap_start_;
  char* heap_end_;

  char* tospace_;
  char* fromspace_;
  size_t extent_;
  char* top_;
  char* free_;

  // Cheney's works list
  void Initialise(void* worklist) { scan_ = free_; }
  bool IsEmpty(void* worklist) { return scan_ == free_; }
  void* Remove(void* worklist) {
    void* ref = static_cast<Header*>(scan_) + 1;
    scan_ = static_cast<char*>(scan_) + Size(ref);
    return ref;
  }
  void Add(void* worklist, void* ref) {}

  void Stats() {
    std::map<HeapObject::Type, size_t> stats;
    char* ptr = tospace_;
    while (ptr != free_) {
      Header* header = reinterpret_cast<Header*>(ptr);
      HeapObject* heap_obj = reinterpret_cast<HeapObject*>(header + 1);
      stats[heap_obj->type()] += header->size;
      ptr += header->size;
    }
    for (auto pair : stats) {
      if (pair.second / 1024 / 1024)
        std::cout << HeapObject::ToString(pair.first) << ": " << pair.second / 1024 / 1024 << " MB" << std::endl;
    }
  }

  void* worklist_ = nullptr;
  void* scan_;
};

}  // namespace es

#endif  // ES_GC_COPYING_COLLECTION
