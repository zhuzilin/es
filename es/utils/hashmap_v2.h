#ifndef ES_UTILS_HASHMAP_V2_H
#define ES_UTILS_HASHMAP_V2_H

#include <queue>
#include <unordered_map>

#include <es/gc/heap_object.h>
#include <es/types/base.h>

namespace es {

// NOTE(zhuzilin) For now, the key type will be String*.
class HashMapV2 : public JSValue {
 public:
  struct Entry {
    String* key;
    JSValue* val;
    uint32_t hash;
  };

  static_assert(sizeof(Entry) == 24);

  static Handle<HashMapV2> New(size_t capacity = kDefaultHashMapSize) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "HashMapV2::New" << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New(kElementOffset + capacity * sizeof(Entry) - HeapObject::kHeapObjectOffset);

    SET_VALUE(jsval.val(), kCapacityOffset, capacity, uint32_t);
    SET_VALUE(jsval.val(), kOccupancyOffset, 0, uint32_t);

    Entry* map = TYPED_PTR(jsval.val(), kElementOffset, Entry);
    for (size_t i = 0; i < capacity; i++) {
      map[i].key = nullptr;
    }

    jsval.val()->SetType(HASHMAP_V2);
    return Handle<HashMapV2>(jsval);
  }

  uint32_t occupancy() { return READ_VALUE(this, kOccupancyOffset, uint32_t); }
  void set_occupancy(size_t s) { SET_VALUE(this, kOccupancyOffset, s, uint32_t); }
  uint32_t capacity() { return READ_VALUE(this, kCapacityOffset, uint32_t); }

  Entry* map_start() { return TYPED_PTR(this, kElementOffset, Entry); }
  const Entry* map_end() { return TYPED_PTR(this, kElementOffset, Entry) + capacity(); }

  // Set can not be method as there can be gc happening inside.
  static Handle<HashMapV2> Set(Handle<HashMapV2> map, Handle<String> key, Handle<JSValue> val) {
    if (map.val()->occupancy() + map.val()->occupancy() / 4 + 1 >= map.val()->capacity()) {
      map = Resize(map);
    }

    // There will not be any new memory allocated after this line.
    // So we could use pointer.
    uint32_t hash = key.val()->Hash();
    Entry* p = map.val()->Probe(key.val(), hash);
    if (p->key == nullptr) {
      map.val()->set_occupancy(map.val()->occupancy() + 1);
      p->key = key.val();
    }
    p->hash = hash;
    p->val = val.val();
    return map;
  }

  HashMapV2::Entry* Probe(String* key, uint32_t hash) {
    ASSERT(key != NULL);
    size_t cap = capacity();
    size_t occ = occupancy();
    Entry* map = map_start();
    const Entry* end = map_end();
    Entry* p = map + (hash & (cap - 1));
    ASSERT(map <= p && p < end);

    ASSERT(occ < cap);
    while (p->key != nullptr && (hash != p->hash || !StringEqual(key, p->key))) {
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

  JSValue* GetRaw(Handle<String> key) {
    uint32_t hash = key.val()->Hash();
    Entry* p = Probe(key.val(), hash);
    if (p->key == nullptr)
      return nullptr;
    return p->val;
  }

  void Delete(Handle<String> key) {
    uint32_t hash = key.val()->Hash();
    Entry* p = Probe(key.val(), hash);
    if (p->key == nullptr)
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
    p->key = nullptr;
    set_occupancy(occupancy() - 1);
  }

  std::vector<std::pair<String*, JSValue*>> SortedKeyValPairs() {
    std::priority_queue<Entry*, std::vector<Entry*>, CompareListNode> pq;
    uint32_t n = occupancy();
    for (Entry* p = map_start(); n > 0; ++p) {
      if (p->key != nullptr) {
        pq.push(p);
        n--;
      }
    }
    std::vector<std::pair<String*, JSValue*>> result;
    while (!pq.empty()) {
      Entry* p = pq.top();
      pq.pop();
      result.emplace_back(std::make_pair(p->key, p->val));
    }
    return result;
  }

 private:
  static Handle<HashMapV2> Resize(Handle<HashMapV2> map) {
    Handle<HashMapV2> new_map = HashMapV2::New(2 * map.val()->capacity());

    uint32_t n = map.val()->occupancy();
    new_map.val()->set_occupancy(n);
    // Rehash all current entries.
    for (Entry* p = map.val()->map_start(); n > 0; p++) {
      if (p->key != nullptr) {
        Entry* new_p = new_map.val()->Probe(p->key, p->hash);
        new_p->hash = p->hash;
        new_p->key = p->key;
        new_p->val = p->val;
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

  static constexpr size_t kDefaultHashMapSize = 4;
};

}  // namespace es

#endif  // ES_UTILS_HASHMAP_V2_H