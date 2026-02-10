#ifndef ES_IMPL_LEXICAL_ENVIRONMENT_IMPL
#define ES_IMPL_LEXICAL_ENVIRONMENT_IMPL

#include <es/types/lexical_environment.h>
#include <es/runtime.h>

namespace es {

Handle<Reference> GetIdentifierReference(Handle<EnvironmentRecord> env_rec, Handle<String> name, bool strict) {
  EnvironmentRecord* cur_raw = env_rec.val();
  while (cur_raw != nullptr) {
    if (cur_raw->IsDeclarativeEnv()) {
      // Fast path: check declarative env directly without creating Handle
      auto decl_env = static_cast<DeclarativeEnvironmentRecord*>(cur_raw);
      if (decl_env->bindings()->GetRaw(name) != nullptr) {
        return Runtime::TopContext().AddReference(Handle<EnvironmentRecord>(cur_raw), name);
      }
    } else {
      ASSERT(cur_raw->IsObjectEnv());
      Handle<EnvironmentRecord> cur_handle(cur_raw);
      if (HasBinding__Object(static_cast<Handle<ObjectEnvironmentRecord>>(cur_handle), name)) {
        return Runtime::TopContext().AddReference(cur_handle, name);
      }
    }
    cur_raw = cur_raw->outer_raw();
  }
  return Runtime::TopContext().AddReference(Undefined::Instance(), name);
}

}  // namespace es

#endif  // ES_IMPL_LEXICAL_ENVIRONMENT_IMPL
