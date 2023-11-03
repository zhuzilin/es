#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

#include <es/types/type.h>
#include <es/utils/macros.h>
#include <es/gc/handle.h>

namespace es {

class HeapObject {
 public:
  template<flag_t flag = 0>
  static Handle<HeapObject> New(size_t size);

  template<uint32_t size, flag_t flag = 0>
  static Handle<HeapObject> New();

  inline Type type() { return h_.type; }
  inline void SetType(Type t) { h_.type = t; }

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;
  void* operator new(size_t, void* ptr) = delete;

  static std::vector<HeapObject**> Pointers(HeapObject* heap_obj);

  static std::string ToString(Type type);

 public:
  static_assert(sizeof(Header) == 16);
  static constexpr size_t kHeaderOffset = sizeof(Header);
  static constexpr size_t kHeapObjectOffset = kHeaderOffset;
  Header h_;
};

class HashMapV2;
class DeclarativeEnvironmentRecord;
class ProgramOrFunctionBody;
struct ExtracGC {
  // save the resized hashmap
  static std::unordered_map<uint32_t, std::stack<HashMapV2*>> resize_released_maps;

  struct FunctionDeclarativeEnvironmentRecord {
    size_t id;
    size_t call_count;
    size_t stack_depth;
    size_t num_pushed;
    DeclarativeEnvironmentRecord** env_rec;

    FunctionDeclarativeEnvironmentRecord(size_t id) :
      call_count(0), stack_depth(0), num_pushed(0) {
      env_rec = new DeclarativeEnvironmentRecord*[kMaxNumPushed];
    }

    DeclarativeEnvironmentRecord** operator[](size_t index) {
      return env_rec + 8 * id + index;
    }

    static constexpr size_t kMaxNumPushed = 8;
    static constexpr size_t kMaxFunctionStored = 1024 * 1024;
    static DeclarativeEnvironmentRecord* env_recs[kMaxNumPushed * kMaxFunctionStored];
  };

  static void TrySaveFunctionEnvRec(
    ProgramOrFunctionBody* body, Handle<DeclarativeEnvironmentRecord> env_rec
  );

  static Handle<DeclarativeEnvironmentRecord> TryPopFunctionEnvRec(
    ProgramOrFunctionBody* body
  );

  static std::vector<HeapObject**> Pointers();

  static std::unordered_map<ProgramOrFunctionBody*, FunctionDeclarativeEnvironmentRecord>
    function_env_recs;
  static constexpr size_t kMinFunctionEnvRecSavingThreshold = 3;
};

DeclarativeEnvironmentRecord* ExtracGC::FunctionDeclarativeEnvironmentRecord::env_recs[kMaxNumPushed * kMaxFunctionStored];

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H