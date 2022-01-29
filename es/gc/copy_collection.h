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
    std::cout << "enter CopyCollection::Collect" << std::endl;
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
#ifdef GC_DEBUG
    std::cout << "exit CopyCollection::Collect " << free_ - tospace_ << std::endl;
#endif
  }

  void Flip() {
    std::swap(fromspace_, tospace_);
    top_ = tospace_ + extent_;
    free_ = tospace_;
    memset(tospace_, 0, extent_);
#ifdef GC_DEBUG
    std::cout << "Flip new top: " << top_ - heap_start_ << std::endl;
    std::cout << "Flip new free: " << free_ - heap_start_ << std::endl;
#endif
  }

  void Scan(void* ref) {
    HeapObject* heap_ref = static_cast<HeapObject*>(ref);
    assert(heap_ref != nullptr);
#ifdef GC_DEBUG
    std::cout << "Scanning: " << heap_ref->ToString() << " " << ref << std::endl;
#endif
    auto ref_pointers = heap_ref->Pointers();
    for (HeapObject** fld : ref_pointers) {
      Process(fld);
    }
#ifdef GC_DEBUG
    std::cout << "exit Scan" << std::endl;
#endif
  }

  void Process(HeapObject** fld) {
    if (*fld == nullptr || (Flag(*fld) & GCFlag::CONST))
      return;
    HeapObject* from_ref = *fld;
    assert(from_ref != nullptr);
    if (tospace_ < (void*)from_ref && (void*)from_ref < tospace_ + extent_) {
      return;
    }
    assert(fromspace_ < (char*)from_ref && (char*)from_ref < fromspace_ + extent_);
#ifdef GC_DEBUG
    std::cout << "Processing: " << from_ref->ToString() << std::endl;
#endif
    *fld = static_cast<HeapObject*>(Forward(from_ref));
#ifdef GC_DEBUG
    std::cout << "exit Process" << std::endl;
#endif
  }

  void* Forward(void* from_ref) {
    void* to_ref = ForwardAddress(from_ref);
    if (to_ref == nullptr)
      to_ref = Copy(from_ref);
    assert(tospace_ < to_ref && to_ref < tospace_ + extent_);
    return to_ref;
  }

  void* Copy(void* from_ref) {
    assert(fromspace_ < from_ref && from_ref < fromspace_ + extent_);
    char* to_ref = free_ + sizeof(Header);
    size_t size = Size(from_ref);
#ifdef GC_DEBUG
    std::cout << "copy to free: " << free_ - tospace_ << std::endl;
#endif
    free_ += size;
    SetForwardAddress(from_ref, nullptr);
    memcpy(H(to_ref), H(from_ref), size);
    SetForwardAddress(from_ref, to_ref);
    Add(worklist_, to_ref);
    return to_ref;
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
