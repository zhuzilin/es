#ifndef ES_UTILS_MAP_H
#define ES_UTILS_MAP_H

#include <functional>
#include <queue>
#include <unordered_map>

#include <es/gc/heap_object.h>
#include <es/types/base.h>

namespace es {

std::unordered_map<size_t, size_t> kExpandHashMapSize = {
  {3, 7},
  {7, 17},
  {17, 37},
  {37, 67},
  {67, 67},
};

namespace list_node {

static constexpr size_t kKeyOffset = 0;
static constexpr size_t kValOffset = kKeyOffset + sizeof(JSValue);
static constexpr size_t kNextOffset = kValOffset + sizeof(JSValue);

JSValue New(JSValue key, JSValue val) {
  ASSERT(key.IsString());
  JSValue jsval;
  std::cout << "enter list" << std::endl;
  jsval.handle() = HeapObject::New(3 * sizeof(JSValue));

  SET_JSVALUE(jsval.handle().val(), kKeyOffset, key);
  SET_JSVALUE(jsval.handle().val(), kValOffset, val);
  SET_JSVALUE(jsval.handle().val(), kNextOffset, null::New());

  jsval.SetType(LIST_NODE);
  return jsval;
}

JSValue key(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kKeyOffset); }
JSValue val(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kValOffset); }
void SetVal(JSValue& jsval, JSValue val) { SET_JSVALUE(jsval.handle().val(), kValOffset, val); }
JSValue next(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kNextOffset); }
void SetNext(JSValue& jsval, JSValue next) {
  ASSERT(jsval.type() == LIST_NODE);
  ASSERT(next.type() == LIST_NODE || next.IsNull());
  SET_JSVALUE(jsval.handle().val(), kNextOffset, next);
}

}  // namespace list_node

namespace hash_map {

static constexpr size_t kInlineCacheOffset = 0;
static constexpr size_t kNumBucketOffset = kInlineCacheOffset + sizeof(JSValue);
static constexpr size_t kSizeOffset = kNumBucketOffset + kSizeTSize;
static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;

static constexpr size_t kDefaultHashMapSize = 3;

JSValue New(size_t num_bucket = kDefaultHashMapSize) {
  JSValue jsval;
  std::cout << "enter hash" << std::endl;
  jsval.handle() = HeapObject::New(kElementOffset + num_bucket * sizeof(JSValue));

  SET_JSVALUE(jsval.handle().val(), kInlineCacheOffset, null::New());
  SET_VALUE(jsval.handle().val(), kNumBucketOffset, num_bucket, size_t);
  SET_VALUE(jsval.handle().val(), kSizeOffset, 0, size_t);
  for (size_t i = 0; i < num_bucket; i++) {
    SET_JSVALUE(jsval.handle().val(), kElementOffset + i * sizeof(JSValue), null::New());
  }

  std::cout << "exit hash_map::New " << jsval.handle().val() << std::endl;

  jsval.SetType(HASHMAP);
  return jsval;
}

size_t size(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kSizeOffset, size_t); }
void SetSize(JSValue& jsval, size_t s) { SET_VALUE(jsval.handle().val(), kSizeOffset, s, size_t); }
size_t num_bucket(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kNumBucketOffset, size_t); }

JSValue inline_cache(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kInlineCacheOffset); }
void SetInlineCache(JSValue& jsval, JSValue node) {
  ASSERT(jsval.type() == HASHMAP);
  ASSERT(node.type() == LIST_NODE || node.IsNull());
  SET_JSVALUE(jsval.handle().val(), kInlineCacheOffset, node);
}

inline bool IsIntegerIndices(std::u16string_view a) {
  if (a.size() == 1 && a[0] == u'0') {
    return true;
  }
  if (!character::IsDecimalDigit(a[0]) || a[0] == u'0') {
    return false;
  }
  for (size_t i = 1; i < a.size(); i++) {
    if (!character::IsDecimalDigit(a[0]))
      return false;
  }
  return true;
}

// TODO(zhuzilin) The order of the properties are determined by ES5 spec.
// However, array need to have a ordered property.
// Try to follow the traverse order in ES6
inline bool LessThan(JSValue key_a, JSValue key_b) {
  ASSERT(key_a.IsString() && key_b.IsString());
  std::u16string_view a = string::data_view(key_a);
  std::u16string_view b = string::data_view(key_b);
  bool num_a = IsIntegerIndices(a);
  bool num_b = IsIntegerIndices(b);
  if (num_a && num_b) {
    return a.size() == b.size() ? a < b : a.size() < b.size();
  }
  return num_a == num_b ? a < b : num_a > num_b;
}

inline bool Equal(JSValue key_a, JSValue key_b) {
  ASSERT(key_a.IsString() && key_b.IsString());
  return string::data_view(key_a) == string::data_view(key_b);
}

struct CompareListNode {
  bool operator()(JSValue x, JSValue y) {
    ASSERT(x.type() == LIST_NODE && y.type() == LIST_NODE);
    return !LessThan(list_node::key(x), list_node::key(y));
  }
};

inline JSValue GetListHead(JSValue jsval, size_t offset) {
  return GET_JSVALUE(jsval.handle().val(), offset);
}

inline void SetListHead(JSValue& jsval, size_t offset, JSValue head) {
  ASSERT(head.type() == LIST_NODE || head.IsNull());
  SET_JSVALUE(jsval.handle().val(), offset, head);
}

inline size_t ListHeadOffset(JSValue jsval, JSValue key) {
  return kElementOffset + std::hash<std::u16string_view>{}(string::data(key)) % num_bucket(jsval) * sizeof(JSValue);
}

inline JSValue GetListNodeRaw(JSValue jsval, JSValue key) {
  ASSERT(key.IsString());
  size_t offset = ListHeadOffset(jsval, key);
  JSValue head = GetListHead(jsval, offset);
  if (head.IsNull())
    return null::New();
  while (!head.IsNull()) {
    JSValue head_key = list_node::key(head);
    if (Equal(key, head_key)) {
      return head;
    } else if (LessThan(key, head_key)) {
      return null::New();
    }
    head = list_node::next(head);
  }
  return null::New();
}

inline JSValue Rehash(JSValue map) {
  size_t old_num_bucket = num_bucket(map);
  size_t new_num_bucket = kExpandHashMapSize[old_num_bucket];
  if (new_num_bucket == old_num_bucket)
    return map;
  JSValue new_map = hash_map::New(new_num_bucket);
  for (size_t i = 0; i < old_num_bucket; i++) {
    size_t offset = kElementOffset + i * sizeof(JSValue);
    JSValue node = GetListHead(map, offset);
    while (!node.IsNull()) {
      JSValue next_node = list_node::next(node);
      size_t offset = ListHeadOffset(new_map, list_node::key(node));
      JSValue head = GetListHead(new_map, offset);
      // Need to make sure the list is sorted when rehashing
      if (head.IsNull() || LessThan(list_node::key(node), list_node::key(head))) {
        list_node::SetNext(node, head);
        SetListHead(new_map, offset, node);
      } else {
        ASSERT(!Equal(list_node::key(node), list_node::key(head)));
        bool inserted = false;
        while (!list_node::next(head).IsNull()) {
          JSValue next = list_node::next(head);
          JSValue next_key =  list_node::key(next);
          ASSERT(!Equal(list_node::key(node), next_key));
          if (LessThan(list_node::key(node), next_key)) {
            list_node::SetNext(node, next);
            list_node::SetNext(head, node);
            inserted = true;
            break;
          }
          head = next;
        }
        if (!inserted) {
          list_node::SetNext(head, node);
          list_node::SetNext(node, null::New());
        }
      }
      node = next_node;
    }
  }
  SetSize(new_map, size(map));
  return new_map;
}

// Set can not be method as there can be gc happening inside.
// NOTE: always need to use newly created map to override the origin map.
inline JSValue Set(JSValue map, JSValue key, JSValue val) {
  // inline cache
  JSValue cache = inline_cache(map);
  if (!cache.IsNull() && Equal(list_node::key(cache), key)) {
    list_node::SetVal(cache, val);
    return map;
  }

  JSValue old_node = GetListNodeRaw(map, key);
  if (!old_node.IsNull()) {
    list_node::SetVal(old_node, val);
    // update inline cache
    SetInlineCache(map, old_node);
    return map;
  }

  JSValue new_node = list_node::New(key, val);
  if (size(map) == num_bucket(map)) {
    map = Rehash(map);
  }
  // There will not be any new memory allocated after this line.
  // So we could use pointer.
  SetSize(map, size(map) + 1);
  // update inline cache
  SetInlineCache(map, new_node);
  size_t offset = ListHeadOffset(map, key);
  JSValue head = GetListHead(map, offset);
  if (head.IsNull() ||
      LessThan(key, list_node::key(head))) {
    list_node::SetNext(new_node, head);
    SetListHead(map, offset, new_node);
    return map;
  }
  ASSERT(!Equal(key, list_node::key(head)));
  while (!list_node::next(head).IsNull()) {
    JSValue next = list_node::next(head);
    JSValue next_key = list_node::key(next);
    ASSERT(!Equal(key, next_key));
    if (LessThan(key, next_key)) {
      list_node::SetNext(new_node, next);
      list_node::SetNext(head, new_node);
      return map;
    }
    head = next;
  }
  list_node::SetNext(head, new_node);
  return map;
}

JSValue Get(JSValue map, JSValue key) {
  ASSERT(map.type() == HASHMAP);
  ASSERT(key.IsString());
  // inline cache
  JSValue cache = inline_cache(map);
  if (false && !cache.IsNull() && Equal(list_node::key(cache), key)) {
    return list_node::val(cache);
  }

  JSValue node = GetListNodeRaw(map, key);
  if (node.IsNull())
    return null::New();
  // update inline cache
  SetInlineCache(map, node);
  return list_node::val(node);
}

void Delete(JSValue map, JSValue key) {
  // inline cache
  JSValue cache = inline_cache(map);
  if (!cache.IsNull() && Equal(list_node::key(cache), key)) {
    SetInlineCache(map, null::New());
  }

  size_t offset = ListHeadOffset(map, key);
  JSValue head = GetListHead(map, offset);
  if (head.IsNull())
    return;
  if (Equal(list_node::key(head), key)) {
    SetListHead(map, offset, list_node::next(head));
    SetSize(map, size(map) - 1);
    return;
  }
  while (!list_node::next(head).IsNull()) {
    JSValue next = list_node::next(head);
    if (Equal(key, list_node::key(next))) {
      list_node::SetNext(head, list_node::next(next));
      SetSize(map, size(map) - 1);
      return;
    } else if (LessThan(key, list_node::key(next))) {
      return;
    }
    head = next;
  }
}

std::vector<std::pair<JSValue, JSValue>> SortedKeyValPairs(JSValue map, bool (*filter)(JSValue)) {
  std::priority_queue<JSValue, std::vector<JSValue>, CompareListNode> pq;
  for (size_t i = 0; i < num_bucket(map); i++) {
    size_t offset = kElementOffset + i * sizeof(JSValue);
    JSValue head = GetListHead(map, offset);
    if (!head.IsNull()) {
      pq.push(head);
    }
  }
  std::vector<std::pair<JSValue, JSValue>> result;
  while (!pq.empty()) {
    JSValue node = pq.top();
    pq.pop();
    if (filter(list_node::val(node))) {
      result.emplace_back(std::make_pair(list_node::key(node), list_node::val(node)));
    }
    if (!list_node::next(node).IsNull()) {
      pq.push(list_node::next(node));
    }
  }
  return result;
}

}  // namespace hash_map

}  // namespace es

#endif