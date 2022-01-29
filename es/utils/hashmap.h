#ifndef ES_UTILS_MAP_H
#define ES_UTILS_MAP_H

#include <queue>

#include <es/gc/heap_object.h>
#include <es/types/base.h>

namespace es {

// NOTE(zhuzilin) For now, the key type will be String*.
template<typename T>
class HashMap : public HeapObject {
 public:
  static Handle<HashMap<T>> New(size_t num_bucket = kDefaultHashMapSize) {
    Handle<HeapObject> heap_obj = HeapObject::New(2 * kSizeTSize + num_bucket * kPtrSize);
    SET_VALUE(heap_obj.val(), kNumBucketOffset, num_bucket, size_t);
    SET_VALUE(heap_obj.val(), kSizeOffset, 0, size_t);
    for (size_t i = 0; i < num_bucket; i++) {
      SET_HANDLE_VALUE(heap_obj.val(), kElementOffset + i * kPtrSize, Handle<ListNode>(), ListNode);
    }
    new (heap_obj.val()) HashMap<T>();
    return Handle<HashMap<T>>(heap_obj);
  }

  std::vector<HeapObject**> Pointers() override {
    size_t n = num_bucket();
    std::vector<HeapObject**> pointers(n);
    for (size_t i = 0; i < n; i++) {
      pointers[i] = HEAP_PTR(kElementOffset + i * kPtrSize);
    }
    return pointers;
  }

  size_t size() { return READ_VALUE(this, kSizeOffset, size_t); }
  void SetSize(size_t s) { SET_VALUE(this, kSizeOffset, s, size_t); }
  size_t num_bucket() { return READ_VALUE(this, kNumBucketOffset, size_t); }

  void Set(Handle<String> key, Handle<T> val) {
    size_t offset = ListHeadOffset(key.val());
    ListNode* head = GetListHead(offset);
    if (head == nullptr || LessThan(key.val(), head->key())) {
      Handle<ListNode> new_head = ListNode::New(key, val);
      new_head.val()->SetNext(head);
      SetListHead(offset, new_head.val());
      SetSize(size() + 1);
      return;
    } else if (*key.val() == *(head->key())) {
      head->SetVal(val.val());
      return;
    }
    while (head->next() != nullptr) {
      String* next_key = head->next()->key();
      if (*key.val() == *next_key) {
        head->next()->SetVal(val.val());
        return;
      } else if (LessThan(key.val(), next_key)) {
        Handle<ListNode> node = ListNode::New(key, val);
        node.val()->SetNext(head->next());
        head->SetNext(node.val());
        SetSize(size() + 1);
        return;
      }
      head = head->next();
    }
    head->SetNext(ListNode::New(key, val).val());
    SetSize(size() + 1);
  }

  Handle<T> Get(Handle<String> key) {
    size_t offset = ListHeadOffset(key.val());
    ListNode* head = GetListHead(offset);
    if (head == nullptr)
      return Handle<T>();
    while (head != nullptr) {
      if (*key.val() == *(head->key())) {
        return Handle<T>(head->val());
      } else if (LessThan(key.val(), head->key())) {
        return Handle<T>();
      }
      head = head->next();
    }
    return Handle<T>();
  }

  T* GetRaw(Handle<String> key) {
    size_t offset = ListHeadOffset(key.val());
    ListNode* head = GetListHead(offset);
    if (head == nullptr)
      return nullptr;
    while (head != nullptr) {
      if (*key.val() == *(head->key())) {
        return head->val();
      } else if (LessThan(key.val(), head->key())) {
        return nullptr;
      }
      head = head->next();
    }
    return nullptr;
  }

  void Delete(Handle<String> key) {
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

  std::vector<std::pair<String*, T*>> SortedKeyValPairs(bool (*filter)(T*)) {
    std::priority_queue<ListNode*, std::vector<ListNode*>, CompareListNode> pq;
    for (size_t i = 0; i < num_bucket(); i++) {
      size_t offset = kElementOffset + i * kPtrSize;
      ListNode* head = GetListHead(offset);
      if (head != nullptr) {
        pq.push(head);
      }
    }
    std::vector<std::pair<String*, T*>> result;
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

  std::string ToString() override { return "HashMap(" + std::to_string(size()) + ")"; }

 private:
  class ListNode : public HeapObject {
   public:
    static Handle<ListNode> New(Handle<String> key, Handle<T> val) {
      Handle<HeapObject> heap_obj = HeapObject::New(3 * kPtrSize);
      SET_HANDLE_VALUE(heap_obj.val(), kKeyOffset, key, String);
      SET_HANDLE_VALUE(heap_obj.val(), kValOffset, val, T);
      SET_HANDLE_VALUE(heap_obj.val(), kNextOffset, Handle<ListNode>(), ListNode);
      return Handle<ListNode>(new (heap_obj.val()) ListNode());
    }

    std::vector<HeapObject**> Pointers() override {
      return {HEAP_PTR(kKeyOffset), HEAP_PTR(kValOffset), HEAP_PTR(kNextOffset)};
    }

    String* key() { return READ_VALUE(this, kKeyOffset, String*); }
    T* val() { return READ_VALUE(this, kValOffset, T*); }
    void SetVal(T* val) { SET_VALUE(this, kValOffset, val, T*); }
    ListNode* next() { return READ_VALUE(this, kNextOffset, ListNode*); }
    void SetNext(ListNode* next) { SET_VALUE(this, kNextOffset, next, ListNode*); }

    std::string ToString() override { return "ListNode(" + key()->ToString() + ")"; }

   private:
    static constexpr size_t kKeyOffset = kHeapObjectOffset;
    static constexpr size_t kValOffset = kKeyOffset + kPtrSize;
    static constexpr size_t kNextOffset = kValOffset + kPtrSize;
  };

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
    return kElementOffset + U16Hash(key->data()) % num_bucket() * kPtrSize;
  }

  static constexpr size_t kNumBucketOffset = kHeapObjectOffset;
  static constexpr size_t kSizeOffset = kNumBucketOffset + kSizeTSize;
  static constexpr size_t kElementOffset = kSizeOffset + kSizeTSize;

  static constexpr size_t kDefaultHashMapSize = 7;

  static constexpr std::hash<std::u16string> U16Hash = std::hash<std::u16string>{};
};

}  // namespace es

#endif