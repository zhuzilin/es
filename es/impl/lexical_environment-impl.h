#ifndef ES_IMPL_LEXICAL_ENVIRONMENT_IMPL
#define ES_IMPL_LEXICAL_ENVIRONMENT_IMPL

#include <es/types/lexical_environment.h>
#include <es/runtime.h>

namespace es {

Handle<Reference> GetIdentifierReference(Handle<EnvironmentRecord> env_rec, Handle<String> name, bool strict) {
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return Runtime::TopContext().AddReference(env_rec, name);
  }
  auto outer = env_rec.val()->outer();
  if (outer.IsNullptr()) {
    return Runtime::TopContext().AddReference(Undefined::Instance(), name);
  }
  return GetIdentifierReference(outer, name, strict);
}

}  // namespace es

#endif  // ES_IMPL_LEXICAL_ENVIRONMENT_IMPL
