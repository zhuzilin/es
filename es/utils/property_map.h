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

    Handle<JSValue> jsval = HeapObject::New(2 * kUint32Size + 2 * kPtrSize);

    Handle<FixedArray> fixed_array;
    if (num_fixed_slots > 0)
      fixed_array = FixedArray::New(num_fixed_slots);
    Handle<HashMap> hashmap = HashMap::New();

    SET_VALUE(jsval.val(), kNumFixedSlotsOffset, num_fixed_slots, uint32_t);
    SET_VALUE(jsval.val(), kFixedSlotsBitMaskOffset, 0, uint32_t);
    SET_HANDLE_VALUE(jsval.val(), kFixedArrayOffset, fixed_array, FixedArray);
    SET_HANDLE_VALUE(jsval.val(), kHashMapOffset, hashmap, HashMap);

    jsval.val()->SetType(PROPERTY_MAP);
    return Handle<PropertyMap>(jsval);
  }

  uint32_t num_fixed_slots() { return READ_VALUE(this, kNumFixedSlotsOffset, uint32_t); }
  bool HasBitMask(uint32_t index) {
    uint32_t bitmask = READ_VALUE(this, kFixedSlotsBitMaskOffset, uint32_t);
    return bitmask & (1 << index);
  }
  void SetBitMask(uint32_t index) {
    uint32_t bitmask = READ_VALUE(this, kFixedSlotsBitMaskOffset, uint32_t);
    SET_VALUE(this, kFixedSlotsBitMaskOffset, bitmask | (1 << index), uint32_t);
  }
  void UnsetBitMask(uint32_t index) {
    uint32_t bitmask = READ_VALUE(this, kFixedSlotsBitMaskOffset, uint32_t);
    SET_VALUE(this, kFixedSlotsBitMaskOffset, bitmask & (~(1 << index)), uint32_t);
  }

  Handle<FixedArray> fixed_array() { return READ_HANDLE_VALUE(this, kFixedArrayOffset, FixedArray); }
  Handle<HashMap> hashmap() { return READ_HANDLE_VALUE(this, kHashMapOffset, HashMap); }
  void SetHashMap(Handle<HashMap> hashmap) { SET_HANDLE_VALUE(this, kHashMapOffset, hashmap, HashMap); }

  // Set can not be method as there can be gc happening inside.
  static void Set(Handle<PropertyMap> map, Handle<String> key, Handle<JSValue> val) {
    if (map.val()->IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      auto array = map.val()->fixed_array();
      array.val()->Set(index, val);
      map.val()->SetBitMask(index);
      return;
    }
    auto hashmap = map.val()->hashmap();
    hashmap = HashMap::Set(hashmap, key, val);
    map.val()->SetHashMap(hashmap);
  }

  Handle<JSValue> Get(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      if (HasBitMask(index)) {
        return fixed_array().val()->Get(index);
      } else {
        return Handle<JSValue>();
      }
    }
    return hashmap().val()->Get(key);
  }

  JSValue* GetRaw(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      if (HasBitMask(index)) {
        return fixed_array().val()->GetRaw(index);
      } else {
        return nullptr;
      }
    }
    return hashmap().val()->GetRaw(key);
  }

  void Delete(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      if (HasBitMask(index)) {
        fixed_array().val()->Set(index, Handle<JSValue>());
        UnsetBitMask(index);
      }
      return;
    }
    hashmap().val()->Delete(key);
  }

  std::vector<std::pair<String*, JSValue*>> SortedKeyValPairs(bool (*filter)(JSValue*)) {
    std::vector<std::pair<String*, JSValue*>> result;
    auto array = fixed_array();
    for (uint32_t i = 0; i < num_fixed_slots(); ++i) {
      if (HasBitMask(i)) {
        result.emplace_back(std::make_pair(String::New(i).val(), array.val()->GetRaw(i)));
      }
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
  static constexpr size_t kFixedSlotsBitMaskOffset = kNumFixedSlotsOffset + kUint32Size;
  static constexpr size_t kFixedArrayOffset = kFixedSlotsBitMaskOffset + kUint32Size;
  static constexpr size_t kHashMapOffset = kFixedArrayOffset + kPtrSize;

  static constexpr size_t kMaxPropertyMapSize = 32;
};

}  // namespace es

#endif