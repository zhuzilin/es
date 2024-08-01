#ifndef ES_UTILS_HASHMAP_V2_H
#define ES_UTILS_HASHMAP_V2_H

#include <queue>
#include <unordered_map>

#include <es/gc/heap_object.h>
#include <es/types/base.h>

namespace es {

bool IsPowerOf2(uint32_t v) {
  return !(v & (v - 1));
}

// https://stackoverflow.com/a/466242/5163915
uint32_t NextPowerOf2(uint32_t v) {
  if (IsPowerOf2(v))
    return v;
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

// NOTE(zhuzilin) For now, the key type will be String*.
class HashMapV2 : public JSValue {
 public:
  struct Entry {
    String* key;
    JSValue* val;
    uint32_t hash;
    union {
      struct {
        bool has_writable;
        bool has_configurable;
        bool has_enumerable;
        bool writable : 1;
        bool configurable : 1;
        bool enumerable : 1;
      };
      struct {
        bool can_delete;
        bool is_mutable;
      };
      uint32_t meta_;
    };
    bool is_empty() { return key == NULL; }
  };

  static_assert(sizeof(Entry) == 24);

  static void DoNothing(Entry* entry) {}
  static JSValue* ReturnValue(Entry* entry) { return entry->val; }

  template<size_t kDefaultHashMapSize = 4>
  static Handle<HashMapV2> New(size_t guessed_occupancy = 0) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "HashMapV2::New" << "\n";
#endif
    size_t capacity = NextPowerOf2(guessed_occupancy + guessed_occupancy / 4 + 1);
    if (capacity < kDefaultHashMapSize)
      capacity = kDefaultHashMapSize;
    assert(IsPowerOf2(capacity));

    if (ExtracGC::resize_released_maps[capacity].size()) {
      // no memory allocation
      Handle<HashMapV2> jsval(ExtracGC::resize_released_maps[capacity].top());
      ExtracGC::resize_released_maps[capacity].pop();
      jsval.val()->Clear();
      return jsval;
    }

    Handle<HashMapV2> jsval = HeapObject::New(kElementOffset + capacity * sizeof(Entry) - HeapObject::kHeapObjectOffset);

    SET_VALUE(jsval.val(), kCapacityOffset, capacity, uint32_t);

    jsval.val()->SetType(HASHMAP_V2);
    // do not need to set to 0 as the heap is already 0ed.
    //jsval.val()->Clear();
    return jsval;
  }

  uint32_t occupancy() { return READ_VALUE(this, kOccupancyOffset, uint32_t); }
  void set_occupancy(size_t s) { SET_VALUE(this, kOccupancyOffset, s, uint32_t); }
  uint32_t capacity() { return READ_VALUE(this, kCapacityOffset, uint32_t); }

  Entry* map_start() { return TYPED_PTR(this, kElementOffset, Entry); }
  const Entry* map_end() { return TYPED_PTR(this, kElementOffset, Entry) + capacity(); }

  // Set can not be method as there can be gc happening inside.
  template<typename EntryFn = decltype(DoNothing)>
  static Handle<HashMapV2> Set(
    Handle<HashMapV2> map, Handle<String> key, Handle<JSValue> val, EntryFn entry_fn = DoNothing
  ) {
    if (map.val()->occupancy() + map.val()->occupancy() / 4 + 1 >= map.val()->capacity()) {
      Handle<HashMapV2> new_map = Resize(map);
      ExtracGC::resize_released_maps[map.val()->capacity()].push(map.val());
      map = new_map;
    }

    // There will not be any new memory allocated after this line.
    // So we could use pointer.
    uint32_t hash = key.val()->Hash();
    Entry* p = map.val()->Probe(key.val(), hash);
    if (p->is_empty()) {
      map.val()->set_occupancy(map.val()->occupancy() + 1);
      p->key = key.val();
    }
    p->hash = hash;
    p->val = val.val();
    entry_fn(p);
    return map;
  }

  // Return when there exists one.
  template<typename EntryFn = decltype(DoNothing)>
  static Handle<HashMapV2> Create(
    Handle<HashMapV2> map, Handle<String> key, Handle<JSValue> val, bool& created, EntryFn entry_fn = DoNothing
  ) {
    if (map.val()->occupancy() + map.val()->occupancy() / 4 + 1 >= map.val()->capacity()) {
      Handle<HashMapV2> new_map = Resize(map);
      ExtracGC::resize_released_maps[map.val()->capacity()].push(map.val());
      map = new_map;
    }

    // There will not be any new memory allocated after this line.
    // So we could use pointer.
    uint32_t hash = key.val()->Hash();
    Entry* p = map.val()->Probe(key.val(), hash);
    // return when exists one.
    created = p->is_empty();
    if (!p->is_empty())
      return map;

    if (p->is_empty()) {
      map.val()->set_occupancy(map.val()->occupancy() + 1);
      p->key = key.val();
    }
    p->hash = hash;
    p->val = val.val();
    entry_fn(p);
    return map;
  }

  HashMapV2::Entry* Probe(String* key, uint32_t hash) {
    ASSERT(key != NULL);
    size_t cap = capacity();
    Entry* map = map_start();
    const Entry* end = map_end();
    Entry* p = map + (hash & (cap - 1));
    ASSERT(map <= p && p < end);

#ifdef TEST
    size_t occ = occupancy();
    ASSERT(occ < cap);
#endif
    while (!p->is_empty() && (hash != p->hash || !StringEqual(key, p->key))) {
      p++;
      if (p >= end) {
        p = map;
      }
    }

    return p;
  }

  Handle<JSValue> Get(Handle<String> key) {
    return Handle<JSValue>(GetRaw(key));
  }

  template<typename EntryFn = decltype(DoNothing)>
  JSValue* GetRaw(Handle<String> key, EntryFn entry_fn = DoNothing) {
    uint32_t hash = key.val()->Hash();
    Entry* p = Probe(key.val(), hash);
    if (p->is_empty())
      return nullptr;
    entry_fn(p);
    return p->val;
  }

  Entry* GetEntry(Handle<String> key) {
    uint32_t hash = key.val()->Hash();
    Entry* p = Probe(key.val(), hash);
    if (p->is_empty())
      return nullptr;
    return p;
  }

  void Delete(Handle<String> key) {
    uint32_t hash = key.val()->Hash();
    Entry* p = Probe(key.val(), hash);
    if (p->is_empty())
      return;

    Entry* map = map_start();
    const Entry* end = map_end();
    uint32_t cap = capacity();

    // The following is from v8.

    // To remove an entry we need to ensure that it does not create an empty
    // entry that will cause the search for another entry to stop too soon. If all
    // the entries between the entry to remove and the next empty slot have their
    // initial position inside this interval, clearing the entry to remove will
    // not break the search. If, while searching for the next empty entry, an
    // entry is encountered which does not have its initial position between the
    // entry to remove and the position looked at, then this entry can be moved to
    // the place of the entry to remove without breaking the search for it. The
    // entry made vacant by this move is now the entry to remove and the process
    // starts over.
    // Algorithm from http://en.wikipedia.org/wiki/Open_addressing.

    // This guarantees loop termination as there is at least one empty entry so
    // eventually the removed entry will have an empty entry after it.
    ASSERT(occupancy() < capacity());

    // p is the candidate entry to clear. q is used to scan forwards.
    Entry* q = p;  // Start at the entry to remove.
    while (true) {
      // Move q to the next entry.
      q = q + 1;
      if (q == end) {
        q = map;
      }

      // All entries between p and q have their initial position between p and q
      // and the entry p can be cleared without breaking the search for these
      // entries.
      if (q->key == NULL) {
        break;
      }

      // Find the initial position for the entry at position q.
      Entry* r = map + (q->hash & (cap - 1));

      // If the entry at position q has its initial position outside the range
      // between p and q it can be moved forward to position p and will still be
      // found. There is now a new candidate entry for clearing.
      if ((q > p && (r <= p || r > q)) ||
          (q < p && (r <= p && r > q))) {
        *p = *q;
        p = q;
      }
    }

    // Clear the entry which is allowed to en emptied.
    p->key = NULL;
    set_occupancy(occupancy() - 1);
  }

  template<typename Filter>
  std::vector<Handle<String>> SortedKeys(Filter filter) {
    std::priority_queue<Entry*, std::vector<Entry*>, CompareListNode> pq;
    uint32_t n = occupancy();
    for (Entry* p = map_start(); n > 0; ++p) {
      if (!p->is_empty()) {
        pq.push(p);
        n--;
      }
    }
    std::vector<Handle<String>> result;
    while (!pq.empty()) {
      Entry* p = pq.top();
      pq.pop();
      if (filter(p)) {
        result.emplace_back(p->key);
      }
    }
    return result;
  }

  void Clear() {
    memset(PTR(this, kOccupancyOffset), 0, capacity() * sizeof(Entry) + kUint32Size);
  }

 private:
  static Handle<HashMapV2> Resize(Handle<HashMapV2> map) {
    Handle<HashMapV2> new_map = HashMapV2::New(map.val()->capacity());

    uint32_t n = map.val()->occupancy();
    new_map.val()->set_occupancy(n);
    // Rehash all current entries.
    for (Entry* p = map.val()->map_start(); n > 0; p++) {
      if (!p->is_empty()) {
        Entry* new_p = new_map.val()->Probe(p->key, p->hash);
        new_p->hash = p->hash;
        new_p->key = p->key;
        new_p->val = p->val;
        new_p->meta_ = p->meta_;
        n--;
      }
    }
    return new_map;
  }


  // TODO(zhuzilin) The order of the properties are determined by ES5 spec.
  // However, array need to have a ordered property.
  // Try to follow the traverse order in ES6
  static bool LessThan(String* a, String* b) {
    bool num_a = a->IsArrayIndex();
    bool num_b = b->IsArrayIndex();
    return num_a == num_b ? StringLessThan(a, b) : num_a > num_b;
  }

  struct CompareListNode {
    bool operator()(Entry* x, Entry* y) {
      return !LessThan(x->key, y->key);
    }
  };

 public:
  static constexpr size_t kCapacityOffset = HeapObject::kHeapObjectOffset;
  static constexpr size_t kOccupancyOffset = kCapacityOffset + kUint32Size;
  static constexpr size_t kElementOffset = kOccupancyOffset + kUint32Size;
};

}  // namespace es

#endif  // ES_UTILS_HASHMAP_V2_H