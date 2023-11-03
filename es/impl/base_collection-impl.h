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
  auto iter = function_env_recs.find(body);
  if (iter == function_env_recs.end()) {
    size_t next_func_id = function_env_recs.size();
    if (next_func_id >= FunctionDeclarativeEnvironmentRecord::kMaxFunctionStored) {
      return;
    }
    function_env_recs.emplace(body, next_func_id);
    iter = function_env_recs.find(body);
  }
  iter->second.stack_depth++;
  if (iter->second.call_count < 10 * kMinFunctionEnvRecSavingThreshold) {
    iter->second.call_count++;
  }
  if (iter->second.call_count < kMinFunctionEnvRecSavingThreshold) {
    return;
  }
  if (iter->second.num_pushed < FunctionDeclarativeEnvironmentRecord::kMaxNumPushed) {
    env_rec.val()->SetOuter(Handle<JSValue>());
    env_rec.val()->bindings()->Clear();
    *iter->second[iter->second.num_pushed] = env_rec.val();
    iter->second.num_pushed++;
  }
}

Handle<DeclarativeEnvironmentRecord> ExtracGC::TryPopFunctionEnvRec(
  ProgramOrFunctionBody* body
) {
  auto iter = function_env_recs.find(body);
  if (iter == function_env_recs.end())
    return Handle<DeclarativeEnvironmentRecord>();
  ASSERT(iter->second.stack_depth > 0);
  iter->second.stack_depth--;
  if (iter->second.num_pushed > 0) {
#ifdef GC_DEBUG
    assert(iter->second.env_rec[0]->IsDeclarativeEnv());
#endif
    size_t indx = --iter->second.num_pushed;
    Handle<DeclarativeEnvironmentRecord> env_rec(*(iter->second[indx]));
    return env_rec;
  }
  return Handle<DeclarativeEnvironmentRecord>();
}

std::vector<HeapObject**> ExtracGC::Pointers() {
  std::vector<HeapObject**> pointers;
  for (auto iter = std::begin(function_env_recs); iter != std::end(function_env_recs);) {
    // remove the no longer called functions.
    if (iter->second.call_count < kMinFunctionEnvRecSavingThreshold || iter->second.num_pushed == 0) {
      iter = function_env_recs.erase(iter);
    } else {
      iter->second.call_count = 0;
      for (size_t i = 0; i < iter->second.num_pushed; ++i) {
        pointers.emplace_back(reinterpret_cast<HeapObject**>(iter->second[i]));
      }
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