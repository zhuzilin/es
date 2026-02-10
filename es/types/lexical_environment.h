#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

Handle<EnvironmentRecord> EnvironmentRecord::Global() {
  static Handle<EnvironmentRecord> singleton = ObjectEnvironmentRecord::New(
    Handle<EnvironmentRecord>(), GlobalObject::Instance(), false);
  return singleton;
}

Handle<Reference> GetIdentifierReference(Handle<EnvironmentRecord> lex, Handle<String> name, bool strict);

void GetIdentifierReferenceAndPutValue(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> name, bool strict, Handle<JSValue> value) {
  EnvironmentRecord* cur_raw = env_rec.val();
  while (cur_raw != nullptr) {
    if (cur_raw->IsDeclarativeEnv()) {
      auto decl_env = static_cast<DeclarativeEnvironmentRecord*>(cur_raw);
      uint32_t hash = name.val()->Hash();
      HashMapV2::Entry* p = decl_env->bindings()->Probe(name.val(), hash);
      if (!p->is_empty()) {
        // Found - set value directly (inlined SetMutableBinding)
        if (p->is_mutable) {
          p->val = value.val();
        } else if (strict) {
          e = Error::TypeError(u"set value to immutable binding");
        }
        return;
      }
    } else {
      ASSERT(cur_raw->IsObjectEnv());
      Handle<EnvironmentRecord> cur_handle(cur_raw);
      if (HasBinding__Object(static_cast<Handle<ObjectEnvironmentRecord>>(cur_handle), name)) {
        return PutValueEnvRec(e, cur_handle, name, strict, value);
      }
    }
    cur_raw = cur_raw->outer_raw();
  }
  return PutValueEnvRec(e, Undefined::Instance(), name, strict, value);
}

Handle<JSValue> GetIdentifierReferenceAndGetValue(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> name, bool strict) {
  EnvironmentRecord* cur_raw = env_rec.val();
  while (cur_raw != nullptr) {
    if (cur_raw->IsDeclarativeEnv()) {
      auto decl_env = static_cast<DeclarativeEnvironmentRecord*>(cur_raw);
      uint32_t hash = name.val()->Hash();
      HashMapV2::Entry* p = decl_env->bindings()->Probe(name.val(), hash);
      if (!p->is_empty()) {
        // Found - get value directly (inlined GetBindingValue)
        if (p->val->IsUndefined() && !p->is_mutable) {
          if (strict) {
            e = Error::ReferenceError(name.val()->data() + u" is not defined");
            return Handle<JSValue>();
          }
          return Undefined::Instance();
        }
        return Handle<JSValue>(p->val);
      }
    } else {
      ASSERT(cur_raw->IsObjectEnv());
      Handle<EnvironmentRecord> cur_handle(cur_raw);
      if (HasBinding__Object(static_cast<Handle<ObjectEnvironmentRecord>>(cur_handle), name)) {
        return GetValueEnvRec(e, cur_handle, name, strict);
      }
    }
    cur_raw = cur_raw->outer_raw();
  }
  return GetValueEnvRec(e, Undefined::Instance(), name, strict);
}

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H