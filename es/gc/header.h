#ifndef ES_GC_HEADER_H
#define ES_GC_HEADER_H

#include <stdlib.h>

namespace es {

enum GCFlag {
  CONST   = 1,
  BIG     = 1 << 1,
  SINGLE  = 1 << 2,
  MARK    = 1 << 3,
};

typedef uint8_t flag_t;

struct Header {
  flag_t flag = 0;
  uint32_t size = 0;
  union {
    void* forward_address = nullptr;
    void* next_obj;
  };
};

Header* H(void* ref) { return static_cast<Header*>(ref) - 1; }
flag_t Flag(void* ref) { return H(ref)->flag; }

size_t Size(void* ref) {
  Header* header = H(ref);
  return static_cast<Header*>(header)->size;
}

}  // namespace es

#endif