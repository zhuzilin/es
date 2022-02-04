#ifndef ES_GC_COPY_COLLECTION
#define ES_GC_COPY_COLLECTION

#include <stdlib.h>

#include <algorithm>

#include <es/gc/base.h>
#include <es/utils/helper.h>

namespace es {

class CopyCollection : public GC {
 public:
  CopyCollection(size_t size) {
    heap_start_ = static_cast<char*>(malloc(size));
    memset(heap_start_, 0, size);
    heap_end_ = heap_start_ + size;
    CreateSemispaces();
  }

 private:
  void CreateSemispaces() {
    tospace_ = heap_start_;
    extent_ = (heap_end_ - heap_start_) / 2;
    fromspace_ = heap_start_ + extent_;
    top_ = fromspace_;
    free_ = tospace_;
  }

  void* Allocate(size_t size) override {
    char* result = free_;
    char* newfree = result + size;
    if (newfree > top_)
      return nullptr;
    free_ = newfree;
    return result;
  }

  void Collect() override {
#ifdef GC_DEBUG
    std::cout << "enter CopyCollection::Collect " << free_ - tospace_ << std::endl;
#endif
    Flip();
    Initialise(worklist_);
    auto root_pointers = Runtime::Global()->Pointers();
    assert(root_pointers.size() > 0);
#ifdef GC_DEBUG
    std::cout << "root_pointers size: " << root_pointers.size() << std::endl;
#endif
    for (HeapObject** fld : root_pointers) {
      Process(fld);
    }
#ifdef GC_DEBUG
    std::cout << "finish root_pointers" << std::endl;
#endif
    while (!IsEmpty(worklist_)) {
      void* ref = Remove(worklist_);
      Scan(ref);
    }
    memset(fromspace_, 0, extent_);
    std::cout << "exit CopyCollection::Collect " << (free_ - tospace_) / 1024U / 1024U << std::endl;
  }

  void Flip() {
    std::swap(fromspace_, tospace_);
    top_ = tospace_ + extent_;
    free_ = tospace_;
    memset(tospace_, 0, extent_);
  }

  void Scan(void* ref) {
    HeapObject* heap_ref = static_cast<HeapObject*>(ref);
    assert(heap_ref != nullptr);
    auto ref_pointers = heap_ref->Pointers();
    for (HeapObject** fld : ref_pointers) {
      Process(fld);
    }
  }

  void Process(HeapObject** fld) {
    if (*fld == nullptr || (Flag(*fld) & GCFlag::CONST))
      return;
    HeapObject* from_ref = *fld;
#ifdef GC_DEBUG
    assert(from_ref != nullptr);
#endif
    if (InToSpace(from_ref)) {
      return;
    }
#ifdef GC_DEBUG
    assert(InFromSpace(from_ref));
#endif
    *fld = static_cast<HeapObject*>(Forward(from_ref));
  }

  void* Forward(void* from_ref) {
    void* to_ref = ForwardAddress(from_ref);
    if (to_ref == nullptr) {
      to_ref = Copy(from_ref);
    }
#ifdef GC_DEBUG
    assert(InToSpace(to_ref));
#endif
    return to_ref;
  }

  void* Copy(void* from_ref) {
    assert(InFromSpace(from_ref));
    char* to_ref = free_ + sizeof(Header);
    size_t size = Size(from_ref);
    free_ += size;
#ifdef GC_DEBUG
    assert(InToSpace(free_) || free_ == tospace_ + extent_);
    assert(ForwardAddress(from_ref) == nullptr);
#endif
    memcpy(H(to_ref), H(from_ref), size);
    SetForwardAddress(from_ref, to_ref);
#ifdef GC_DEBUG
    assert(ForwardAddress(to_ref) == nullptr);
    assert(InToSpace(ForwardAddress(from_ref)));
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

  void* worklist_ = nullptr;
  void* scan_;
};

}  // namespace es

#endif  // ES_GC_COPY_COLLECTION
