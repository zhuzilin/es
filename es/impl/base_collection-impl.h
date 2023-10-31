#ifndef ES_IMPL_BASE_COLLECTION_IMPL_H
#define ES_IMPL_BASE_COLLECTION_IMPL_H

#include <es/gc/base_collection.h>
#include <es/utils/hashmap_v2.h>

namespace es {

template<typename T>
void GC<T>::CleanUpBeforeCollect() {
  HashMapV2::released_maps_.clear();
}

}  // namespace

#endif