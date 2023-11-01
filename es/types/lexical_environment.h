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

Handle<EnvironmentRecord> NewDeclarativeEnvironment(Handle<EnvironmentRecord> outer, size_t num_decls) {
  return DeclarativeEnvironmentRecord::New(outer, num_decls);
}

Handle<EnvironmentRecord> NewObjectEnvironment(Handle<JSObject> obj, Handle<EnvironmentRecord> outer, bool provide_this) {
  return ObjectEnvironmentRecord::New(outer, obj, provide_this);
}

Handle<Reference> GetIdentifierReference(Handle<EnvironmentRecord> lex, Handle<String> name, bool strict);

void GetIdentifierReferenceAndPutValue(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> name, bool strict, Handle<JSValue> value) {
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return PutValueEnvRec(e, env_rec, name, strict, value);
  }
  auto outer = env_rec.val()->outer();
  if (outer.IsNullptr()) {
    return PutValueEnvRec(e, Undefined::Instance(), name, strict, value);
  }
  return GetIdentifierReferenceAndPutValue(e, outer, name, strict, value);
}

Handle<JSValue> GetIdentifierReferenceAndGetValue(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> name, bool strict) {
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return GetValueEnvRec(e, env_rec, name, strict);
  }
  auto outer = env_rec.val()->outer();
  if (outer.IsNullptr()) {
    return GetValueEnvRec(e, Undefined::Instance(), name, strict);
  }
  return GetIdentifierReferenceAndGetValue(e, outer, name, strict);
}

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H