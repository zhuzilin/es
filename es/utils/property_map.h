#ifndef ES_UTILS_PROPERTY_MAP_H
#define ES_UTILS_PROPERTY_MAP_H

#include <queue>
#include <unordered_map>

#include <es/types/base.h>
#include <es/types/property_descriptor.h>
#include <es/utils/hashmap.h>
#include <es/utils/hashmap_v2.h>
#include <es/utils/fixed_array.h>

namespace es {

// These 2 conversions only happens here.
Handle<JSValue> ToHeap(StackPropertyDescriptor desc) {
  if (desc.IsUndefined())
    return Undefined::Instance();
  auto new_desc = PropertyDescriptor::New();
  if (desc.HasValue()) {
    new_desc.val()->SetValue(desc.Value());
    if (desc.HasWritable())
      new_desc.val()->SetWritable(desc.Writable());
  } else {
    if (desc.HasGet())
      new_desc.val()->SetGet(desc.Get());
    if (desc.HasSet())
      new_desc.val()->SetSet(desc.Set());
  }
  if (desc.HasConfigurable())
    new_desc.val()->SetConfigurable(desc.Configurable());
  if (desc.HasEnumerable())
    new_desc.val()->SetEnumerable(desc.Enumerable());
  return new_desc;
}

StackPropertyDescriptor ToStack(JSValue* val) {
  if (val == nullptr || val->IsUndefined())
    return StackPropertyDescriptor::Undefined();
  ASSERT(val->IsPropertyDescriptor());
  PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(val);
  StackPropertyDescriptor new_desc;
  if (desc->HasValue()) {
    new_desc.SetValue(desc->Value());
    if (desc->HasWritable())
      new_desc.SetWritable(desc->Writable());
  } else {
    if (desc->HasGet())
      new_desc.SetGet(desc->Get());
    if (desc->HasSet())
      new_desc.SetSet(desc->Set());
  }
  if (desc->HasConfigurable())
    new_desc.SetConfigurable(desc->Configurable());
  if (desc->HasEnumerable())
    new_desc.SetEnumerable(desc->Enumerable());
  return new_desc;
}

class PropertyMap : public JSValue {
 public:
  static Handle<PropertyMap> New(size_t num_fixed_slots = 0, size_t guessed_occupancy = 0) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "PropertyMap::New" << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New(kElementOffset + num_fixed_slots * kPtrSize - HeapObject::kHeapObjectOffset);

    Handle<HashMapV2> hashmap = HashMapV2::New(guessed_occupancy);
    SET_VALUE(jsval.val(), kNumFixedSlotsOffset, num_fixed_slots, size_t);
    SET_HANDLE_VALUE(jsval.val(), kHashMapOffset, hashmap, HashMapV2);

    for (size_t i = 0; i < num_fixed_slots; ++i) {
      SET_VALUE(jsval.val(), kElementOffset + i * kPtrSize, nullptr, JSValue*);
    }

    jsval.val()->SetType(PROPERTY_MAP);
    return Handle<PropertyMap>(jsval);
  }

  uint32_t num_fixed_slots() { return READ_VALUE(this, kNumFixedSlotsOffset, uint32_t); }
  Handle<HashMapV2> hashmap() { return READ_HANDLE_VALUE(this, kHashMapOffset, HashMapV2); }
  void SetHashMap(Handle<HashMapV2> hashmap) { SET_HANDLE_VALUE(this, kHashMapOffset, hashmap, HashMapV2); }
  JSValue* GetRawArray(size_t index) {
    ASSERT(index < num_fixed_slots());
    return READ_VALUE(this, kElementOffset + index * kPtrSize, JSValue*);
  }
  void SetRawArray(size_t index, JSValue* val) {
    ASSERT(index < num_fixed_slots());
    SET_VALUE(this, kElementOffset + index * kPtrSize, val, JSValue*);
  }

  // Set can not be method as there can be gc happening inside.
  static void Set(Handle<PropertyMap> map, Handle<String> key, StackPropertyDescriptor desc) {
    if (map.val()->IsSmallArrayIndex(key)) {
      Handle<PropertyDescriptor> val = ToHeap(desc);
      uint32_t index = key.val()->Index();
      map.val()->SetRawArray(index, val.val());
      return;
    }
    auto hashmap = map.val()->hashmap();
    if (desc.IsDataDescriptor()) {
      auto entry_fn = [&desc] (HashMapV2::Entry* p) {
        p->has_writable = desc.HasWritable();
        p->writable = desc.Writable();
        p->has_configurable = desc.HasConfigurable();
        p->configurable = desc.Configurable();
        p->has_enumerable = desc.HasEnumerable();
        p->enumerable = desc.Enumerable();
      };
      ASSERT(desc.HasValue());
      hashmap = HashMapV2::Set(hashmap, key, desc.Value(), entry_fn);
    } else {
      Handle<PropertyDescriptor> val = ToHeap(desc);
      hashmap = HashMapV2::Set(hashmap, key, val);
    }
    map.val()->SetHashMap(hashmap);
  }

  StackPropertyDescriptor Get(Handle<String> key) {
    JSValue* val;
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      val = GetRawArray(index);
      return ToStack(val);
    } else {
      StackPropertyDescriptor desc;
      auto entry_fn = [&desc](HashMapV2::Entry* p) mutable {
        ASSERT(p != nullptr && p->val != nullptr);
        if (p->val->IsPropertyDescriptor()) {
          desc = ToStack(p->val);
        } else {
          desc.SetValue(Handle<JSValue>(p->val));
          if (p->has_writable)
            desc.SetWritable(p->writable);
          if (p->has_configurable)
            desc.SetConfigurable(p->configurable);
          if (p->has_enumerable)
            desc.SetEnumerable(p->enumerable);
        }
      };
      JSValue* val = hashmap().val()->GetRaw(key, entry_fn);
      if (val == nullptr)
        return StackPropertyDescriptor::Undefined();
      return desc;
    }
  }

  void Delete(Handle<String> key) {
    if (IsSmallArrayIndex(key)) {
      uint32_t index = key.val()->Index();
      SetRawArray(index, nullptr);
      return;
    }
    hashmap().val()->Delete(key);
  }

  template<typename Filter>
  std::vector<std::pair<String*, StackPropertyDescriptor>> SortedKeyValPairs(Filter filter) {
    std::vector<std::pair<String*, StackPropertyDescriptor>> result;
    for (uint32_t i = 0; i < num_fixed_slots(); ++i) {
      StackPropertyDescriptor val = ToStack(GetRawArray(i));
      if (!val.IsUndefined())
        result.emplace_back(std::make_pair(String::New(i).val(), val));
    }
    auto entry_fn = [](HashMapV2::Entry* p) -> StackPropertyDescriptor {
      if (p->key == nullptr) {
        return StackPropertyDescriptor::Undefined();
      }
      ASSERT(p->val != nullptr);
      if (p->val->IsPropertyDescriptor()) {
        return ToStack(p->val);
      } else {
        StackPropertyDescriptor desc;
        desc.SetValue(Handle<JSValue>(p->val));
        if (p->has_writable)
          desc.SetWritable(p->writable);
        if (p->has_configurable)
          desc.SetConfigurable(p->configurable);
        if (p->has_enumerable)
          desc.SetEnumerable(p->enumerable);
        return desc;
      }
    };
    std::vector<std::pair<String*, StackPropertyDescriptor>> hashmap_result =
      hashmap().val()->SortedKeyValPairs<StackPropertyDescriptor>(entry_fn);
    for (auto pair : hashmap_result) {
      if (filter(pair.second))
        result.emplace_back(pair.first, pair.second);
    }
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
};

}  // namespace es

#endif