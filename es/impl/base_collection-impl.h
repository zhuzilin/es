#ifndef ES_IMPL_BASE_COLLECTION_IMPL_H
#define ES_IMPL_BASE_COLLECTION_IMPL_H

#include <es/gc/base_collection.h>
#include <es/utils/hashmap_v2.h>

namespace es {

std::unordered_map<uint32_t, std::stack<HashMapV2*>> ExtracGC::resize_released_maps;
std::unordered_map<ProgramOrFunctionBody*, ExtracGC::FunctionDeclarativeEnvironmentRecord>
    ExtracGC::function_env_recs;

void ExtracGC::TrySaveFunctionEnvRec(
  ProgramOrFunctionBody* body, Handle<DeclarativeEnvironmentRecord> env_rec
) {
  ASSERT(!env_rec.IsNullptr());
  ASSERT(env_rec.val()->IsDeclarativeEnv());
  if (env_rec.val()->ref_count()) {
    // env_rec is still referenced, maybe due to closure.
    return;
  }
  FunctionDeclarativeEnvironmentRecord& function_env_rec = function_env_recs[body];
  function_env_rec.stack_depth++;
  if (function_env_rec.call_count < 10 * kMinFunctionEnvRecSavingThreshold) {
    function_env_rec.call_count++;
  }
  if (function_env_rec.call_count < kMinFunctionEnvRecSavingThreshold) {
    return;
  }
  if (function_env_rec.env_rec[0] == nullptr) {
    env_rec.val()->SetOuter(Handle<JSValue>());
    env_rec.val()->bindings()->Clear();
    function_env_rec.env_rec[0] = env_rec.val();
  }
}

Handle<DeclarativeEnvironmentRecord> ExtracGC::TryPopFunctionEnvRec(
  ProgramOrFunctionBody* body
) {
  auto iter = function_env_recs.find(body);
  if (iter == function_env_recs.end())
    return Handle<DeclarativeEnvironmentRecord>();
  iter->second.stack_depth--;
  if (iter->second.env_rec[0] != nullptr) {
#ifdef GC_DEBUG
    assert(iter->second.env_rec[0]->IsDeclarativeEnv());
#endif
    Handle<DeclarativeEnvironmentRecord> env_rec(iter->second.env_rec[0]);
    iter->second.env_rec[0] = nullptr;
    return env_rec;
  }
  return Handle<DeclarativeEnvironmentRecord>();
}

std::vector<HeapObject**> ExtracGC::Pointers() {
  std::vector<HeapObject**> pointers;
  for (auto iter = std::begin(function_env_recs); iter != std::end(function_env_recs);) {
    // remove the no longer called functions.
    if (iter->second.call_count < kMinFunctionEnvRecSavingThreshold || iter->second.env_rec[0] == nullptr) {
      iter->second.destruct();
      iter = function_env_recs.erase(iter);
    } else {
      iter->second.call_count = 0;
      pointers.emplace_back(reinterpret_cast<HeapObject**>(iter->second.env_rec));
      ++iter;
    }
  }
  return pointers;
}

template<typename T>
void GC<T>::CleanUpBeforeCollect() {
  ExtracGC::resize_released_maps.clear();
}

}  // namespace

#endif