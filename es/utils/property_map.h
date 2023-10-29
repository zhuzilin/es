#ifndef ES_UTILS_PROPERTY_MAP_H
#define ES_UTILS_PROPERTY_MAP_H

#include <queue>
#include <unordered_map>

#include <es/types/base.h>
#include <es/utils/hashmap.h>
#include <es/utils/fixed_array.h>

namespace es {

class PropertyMap : public JSValue {
 public:
  static Handle<PropertyMap> New(size_t num_fixed_slots = 0) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "PropertyMap::New" << "\n";
#endif
    if (num_fixed_slots > kMaxPropertyMapSize)
      num_fixed_slots = kMaxPropertyMapSize;

    Handle<JSValue> jsval = HeapObject::New(kElementOffset + num_fixed_slots * kPtrSize - HeapObject::kHeapObjectOffset);

    Handle<HashMap> hashmap = HashMap::New();
    SET_VALUE(jsval.val(), kNumFixedSlotsOffset, num_fixed_slots, size_t);
    SET_HANDLE_VALUE(jsval.val(), kHashMapOffset, hashmap, HashMap);

    for (size_t i = 0; i < num_fixed_slots; ++i) {
      SET_VALUE(jsval.val(), kElementOffset + i * kPtrSize, nullptr, JSValue*);
    }

    jsval.val()->SetType(PROPERTY_MAP);
    return Handle<PropertyMap>(jsval);
  }

  uint32_t num_fixed_slots() { return READ_VALUE(this, kNumFixedSlotsOffset, uint32_t); }
  Handle<HashMap> hashmap() { return READ_HANDLE_VALUE(this, kHashMapOffset, HashMap); }
  void SetHashMap(Handle<HashMap> hashmap) { SET_HANDLE_VALUE(this, kHashMapOffset, hashmap, HashMap); }
  JSValue* GetRawArray(size_t index) {
    ASSERT(index < num_fixed_slots());
    return READ_VALUE(this, kElementOffset + index * kPtrSize, JSValue*);
  }
  void SetRawArray(size_t index, JSValue* val) {
    ASSERT(index < num_fixed_slots());
    SET_VALUE(this, kElementOffset + index * kPtrSize, val, JSValue*);
  }

  // Set can not be method as there can be gc happening inside.
  static void Set(Handle<PropertyMap> map, Handle<String> key, Handle<JSValue> val) {
    if (map.val()->IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      map.val()->SetRawArray(index, val.val());
      return;
    }
    auto hashmap = map.val()->hashmap();
    hashmap = HashMap::Set(hashmap, key, val);
    map.val()->SetHashMap(hashmap);
  }

  Handle<JSValue> Get(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      return Handle<JSValue>(GetRawArray(index));
    }
    return hashmap().val()->Get(key);
  }

  JSValue* GetRaw(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      return GetRawArray(index);
    }
    return hashmap().val()->GetRaw(key);
  }

  void Delete(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      SetRawArray(index, nullptr);
      return;
    }
    hashmap().val()->Delete(key);
  }

  std::vector<std::pair<String*, JSValue*>> SortedKeyValPairs(bool (*filter)(JSValue*)) {
    std::vector<std::pair<String*, JSValue*>> result;
    for (uint32_t i = 0; i < num_fixed_slots(); ++i) {
      JSValue* val = GetRawArray(i);
      if (val != nullptr)
        result.emplace_back(std::make_pair(String::New(i).val(), GetRawArray(i)));
    }
    std::vector<std::pair<String*, JSValue*>> hashmap_result = hashmap().val()->SortedKeyValPairs(filter);
    result.insert(result.end(), hashmap_result.begin(), hashmap_result.end());
    return result;
  }

 private:
  bool IsSmallArrayIndex(Handle<String> key) {
    return key.val()->IsArrayIndex() && key.val()->Index() < num_fixed_slots();
  }

 public:
  static constexpr size_t kNumFixedSlotsOffset = HeapObject::kHeapObjectOffset;
  static constexpr size_t kHashMapOffset = kNumFixedSlotsOffset + kSizeTSize;
  static constexpr size_t kElementOffset = kHashMapOffset + kPtrSize;

  static constexpr size_t kMaxPropertyMapSize = 128;
};

}  // namespace es

#endif