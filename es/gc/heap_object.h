#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

namespace es {

class HeapObject {
 public:
  virtual std::vector<void*> Pointers() = 0; 
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H