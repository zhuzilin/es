#ifndef ES_UTILS_MAP_H
#define ES_UTILS_MAP_H

#include <queue>
#include <unordered_map>

#include <es/gc/heap_object.h>
#include <es/types/base.h>

namespace es {

class ListNode : public JSValue {
  public:
  static Handle<ListNode> New(Handle<String> key, Handle<JSValue> val) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "ListNode::New" << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New(3 * kPtrSize);

    SET_HANDLE_VALUE(jsval.val(), kKeyOffset, key, String);
    SET_HANDLE_VALUE(jsval.val(), kValOffset, val, JSValue);
    SET_HANDLE_VALUE(jsval.val(), kNextOffset, Handle<ListNode>(), ListNode);
    jsval.val()->SetType(LIST_NODE);
    return Handle<ListNode>(jsval);
  }

  String* key() { return READ_VALUE(this, kKeyOffset, String*); }
  JSValue* val() { return READ_VALUE(this, kValOffset, JSValue*); }
  void SetVal(JSValue* val) { SET_VALUE(this, kValOffset, val, JSValue*); }
  ListNode* next() { return READ_VALUE(this, kNextOffset, ListNode*); }
  void SetNext(ListNode* next) { SET_VALUE(this, kNextOffset, next, ListNode*); }

  public:
  static constexpr size_t kKeyOffset = HeapObject::kHeapObjectOffset;
  static constexpr size_t kValOffset = kKeyOffset + kPtrSize;
  static constexpr size_t kNextOffset = kValOffset + kPtrSize;
};

// NOTE(zhuzilin) For now, the key type will be String*.
class HashMap : public JSValue {
 public:
  static Handle<HashMap> New(size_t num_bucket = kDefaultHashMapSize) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "HashMap::New" << "\n";
#endif
    Handle<JSValue> jsval = HeapObject::New(kElementOffset + num_bucket * kPtrSize - HeapObject::kHeapObjectOffset);

    SET_VALUE(jsval.val(), kInlineCacheOffset, nullptr, ListNode*);
    SET_VALUE(jsval.val(), kNumBucketOffset, num_bucket, size_t);
    SET_VALUE(jsval.val(), kSizeOffset, 0, size_t);
    for (size_t i = 0; i < num_bucket; i++) {
      SET_HANDLE_VALUE(jsval.val(), kElementOffset + i * kPtrSize, Handle<ListNode>(), ListNode);
    }

    jsval.val()->SetType(HASHMAP);
    return Handle<HashMap>(jsval);
  }

  size_t size() { return READ_VALUE(this, kSizeOffset, size_t); }
  void SetSize(size_t s) { SET_VALUE(this, kSizeOffset, s, size_t); }
  size_t num_bucket() { return READ_VALUE(this, kNumBucketOffset, size_t); }

  ListNode* inline_cache() { return READ_VALUE(this, kInlineCacheOffset, ListNode*); }
  void SetInlineCache(ListNode* node) { SET_VALUE(this, kInlineCacheOffset, node, ListNode*); }

  // Set can not be method as there can be gc happening inside.
  static Handle<HashMap> Set(Handle<HashMap> map, Handle<String> key, Handle<JSValue> val) {
    // inline cache
    ListNode* cache = map.val()->inline_cache();
    if (cache != nullptr && *cache->key() == *key.val()) {
      cache->SetVal(val.val());
      return map;
    }

    ListNode* old_node = map.val()->GetListNodeRaw(key);
    if (old_node != nullptr) {
      old_node->SetVal(val.val());
      // update inline cache
      map.val()->SetInlineCache(old_node);
      return map;
    }

    Handle<ListNode> new_node = ListNode::New(key, val);
    if (map.val()->size() == map.val()->num_bucket()) {
      map = Rehash(map);
    }
    // There will not be any new memory allocated after this line.
    // So we could use pointer.
    map.val()->SetSize(map.val()->size() + 1);
    // update inline cache
    map.val()->SetInlineCache(new_node.val());
    size_t offset = map.val()->ListHeadOffset(key.val());
    ListNode* head = map.val()->GetListHead(offset);
    if (head == nullptr || LessThan(key.val(), head->key())) {
      new_node.val()->SetNext(head);
      map.val()->SetListHead(offset, new_node.val());
      return map;
    }
    ASSERT(*key.val() != *(head->key()));
    while (head->next() != nullptr) {
      String* next_key = head->next()->key();
      ASSERT(*key.val() != *next_key);
      if (LessThan(key.val(), next_key)) {
        new_node.val()->SetNext(head->next());
        head->SetNext(new_node.val());
        return map;
      }
      head = head->next();
    }
    head->SetNext(new_node.val());
    return map;
  }

  static Handle<HashMap> Rehash(Handle<HashMap> map) {
    size_t old_num_bucket = map.val()->num_bucket();
    size_t new_num_bucket = (old_num_bucket + 1) * 2 - 1;
    Handle<HashMap> new_map = HashMap::New(new_num_bucket);
    for (size_t i = 0; i < old_num_bucket; i++) {
      size_t offset = kElementOffset + i * kPtrSize;
      ListNode* node = map.val()->GetListHead(offset);
      while (node != nullptr) {
        ListNode* next_node = node->next();
        size_t offset = new_map.val()->ListHeadOffset(node->key());
        ListNode* head = new_map.val()->GetListHead(offset);
        // Need to make sure the list is sorted when rehashing
        if (head == nullptr || LessThan(node->key(), head->key())) {
          node->SetNext(head);
          new_map.val()->SetListHead(offset, node);
        } else {
          ASSERT(*node->key() != *(head->key()));
          bool inserted = false;
          while (head->next() != nullptr) {
            String* next_key = head->next()->key();
            ASSERT(*node->key() != *next_key);
            if (LessThan(node->key(), next_key)) {
              node->SetNext(head->next());
              head->SetNext(node);
              inserted = true;
              break;
            }
            head = head->next();
          }
          if (!inserted) {
            head->SetNext(node);
            node->SetNext(nullptr);
          }
        }
        node = next_node;
      }
    }
    new_map.val()->SetSize(map.val()->size());
    return new_map;
  }

  Handle<JSValue> Get(Handle<String> key) {
    return Handle<JSValue>(GetRaw(key));
  }

  JSValue* GetRaw(Handle<String> key) {
    // inline cache
    ListNode* cache = inline_cache();
    if (cache != nullptr && *cache->key() == *key.val()) {
      return cache->val();
    }

    ListNode* node = GetListNodeRaw(key);
    if (node == nullptr)
      return nullptr;
    // update inline cache
    SetInlineCache(node);
    return node->val();
  }

  void Delete(Handle<String> key) {
    // inline cache
    ListNode* cache = inline_cache();
    if (cache != nullptr && *cache->key() == *key.val()) {
      SetInlineCache(nullptr);
    }

    size_t offset = ListHeadOffset(key.val());
    ListNode* head = GetListHead(offset);
    if (head == nullptr)
      return;
    if (*(head->key()) == *key.val()) {
      SetListHead(offset, head->next());
      SetSize(size() - 1);
      return;
    }
    while (head->next() != nullptr) {
      if (*key.val() == *(head->next()->key())) {
        head->SetNext(head->next()->next());
        SetSize(size() - 1);
        return;
      } else if (LessThan(key.val(), head->next()->key())) {
        return;
      }
      head = head->next();
    }
  }

  std::vector<std::pair<String*, JSValue*>> SortedKeyValPairs(bool (*filter)(JSValue*)) {
    std::priority_queue<ListNode*, std::vector<ListNode*>, CompareListNode> pq;
    for (size_t i = 0; i < num_bucket(); i++) {
      size_t offset = kElementOffset + i * kPtrSize;
      ListNode* head = GetListHead(offset);
      if (head != nullptr) {
        pq.push(head);
      }
    }
    std::vector<std::pair<String*, JSValue*>> result;
    while (!pq.empty()) {
      ListNode* node = pq.top();
      pq.pop();
      if (filter(node->val())) {
        result.emplace_back(std::make_pair(node->key(), node->val()));
      }
      if (node->next() != nullptr) {
        pq.push(node->next());
      }
    }
    return result;
  }

 private:
  static bool IsIntegerIndices(String& a) {
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
  static bool LessThan(String* a, String* b) {
    bool num_a = IsIntegerIndices(*a);
    bool num_b = IsIntegerIndices(*b);
    if (num_a && num_b) {
      return a->size() == b->size() ? *a < *b : a->size() < b->size();
    }
    return num_a == num_b ? *a < *b : num_a > num_b;
  }

  struct CompareListNode {
    bool operator()(ListNode* x, ListNode* y) {
      return !LessThan(x->key(), y->key());
    }
  };

  ListNode* GetListHead(size_t offset) {
    return READ_VALUE(this, offset, ListNode*);
  }

  void SetListHead(size_t offset, ListNode* head) {
    SET_VALUE(this, offset, head, ListNode*);
  }

  inline size_t ListHeadOffset(String* key) {
    return kElementOffset + key->Hash() % num_bucket() * kPtrSize;
  }

  ListNode* GetListNodeRaw(Handle<String> key) {
    size_t offset = ListHeadOffset(key.val());
    ListNode* head = GetListHead(offset);
    if (head == nullptr)
      return nullptr;
    while (head != nullptr) {
      if (*key.val() == *(head->key())) {
        return head;
      } else if (LessThan(key.val(), head->key())) {
        return nullptr;
      }
      head = head->next();
    }
    return nullptr;
  }

 public:
  static constexpr size_t kInlineCacheOffset = HeapObject::kHeapObjectOffset;
  static constexpr size_t kNumBucketOffset = kInlineCacheOffset + kPtrSize;
  static constexpr size_t kSizeOffset = kNumBucketOffset + kSizeTSize;
  static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;

  static constexpr size_t kDefaultHashMapSize = 3;
};

}  // namespace es

#endif