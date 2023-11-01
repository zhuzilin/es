#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

class LexicalEnvironment : public JSValue {
 public:
  static Handle<LexicalEnvironment> New(Handle<LexicalEnvironment> outer, Handle<EnvironmentRecord> env_rec) {
    Handle<LexicalEnvironment> jsval = HeapObject::New<2 * kPtrSize>();

    SET_HANDLE_VALUE(jsval.val(), kOuterOffset, outer, LexicalEnvironment);
    SET_HANDLE_VALUE(jsval.val(), kEnvRecOffset, env_rec, EnvironmentRecord);

    jsval.val()->SetType(JS_LEX_ENV);
    jsval.val()->AddRefCount();
    return jsval;
  }

  void AddRefCount() {
    ASSERT(!env_rec().IsNullptr());
    size_t old_rc = env_rec().val()->ref_count();
    env_rec().val()->AddRefCount();
    // First use, add 1 to outer env_rec.
    if (old_rc == 0) {
      if (unlikely(outer().IsNullptr())) {
        return;
      }
      outer().val()->AddRefCount();
    }
    
  }

  // Reduce the ref_count of outer lexical env.
  void ReduceRefCount() {
    size_t rc = env_rec().val()->ReduceRefCount();
    // No longer use, reduce 1 to outer env_rec
    if (rc == 0) {
      if (!outer().IsNullptr()) {
        outer().val()->ReduceRefCount();
      }
    }
  }

  Handle<LexicalEnvironment> outer() { return READ_HANDLE_VALUE(this, kOuterOffset, LexicalEnvironment); }
  Handle<EnvironmentRecord> env_rec() { return READ_HANDLE_VALUE(this, kEnvRecOffset, EnvironmentRecord); }

  static Handle<LexicalEnvironment> Global() {
    static Handle<LexicalEnvironment> singleton = LexicalEnvironment::New(
      Handle<LexicalEnvironment>(), ObjectEnvironmentRecord::New(GlobalObject::Instance(), false));
    return singleton;
  }

 public:
  static constexpr size_t kOuterOffset = kJSValueOffset;
  static constexpr size_t kEnvRecOffset = kOuterOffset + kPtrSize;
};

Handle<LexicalEnvironment> NewDeclarativeEnvironment(Handle<LexicalEnvironment> lex, size_t num_decls) {
  Handle<DeclarativeEnvironmentRecord> env_rec = DeclarativeEnvironmentRecord::New(num_decls);
  return LexicalEnvironment::New(lex, env_rec);
}

Handle<LexicalEnvironment> NewObjectEnvironment(Handle<JSObject> obj, Handle<LexicalEnvironment> lex, bool provide_this) {
  Handle<ObjectEnvironmentRecord> env_rec = ObjectEnvironmentRecord::New(obj, provide_this);
  return LexicalEnvironment::New(lex, env_rec);
}

Handle<Reference> GetIdentifierReference(Handle<LexicalEnvironment> lex, Handle<String> name, bool strict);

void GetIdentifierReferenceAndPutValue(Handle<Error>& e, Handle<LexicalEnvironment> lex, Handle<String> name, bool strict, Handle<JSValue> value) {
  auto env_rec = lex.val()->env_rec();
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return PutValueEnvRec(e, env_rec, name, strict, value);
  }
  auto outer = lex.val()->outer();
  if (outer.IsNullptr()) {
    return PutValueEnvRec(e, Undefined::Instance(), name, strict, value);
  }
  return GetIdentifierReferenceAndPutValue(e, outer, name, strict, value);
}

Handle<JSValue> GetIdentifierReferenceAndGetValue(Handle<Error>& e, Handle<LexicalEnvironment> lex, Handle<String> name, bool strict) {
  auto env_rec = lex.val()->env_rec();
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return GetValueEnvRec(e, env_rec, name, strict);
  }
  auto outer = lex.val()->outer();
  if (outer.IsNullptr()) {
    return GetValueEnvRec(e, Undefined::Instance(), name, strict);
  }
  return GetIdentifierReferenceAndGetValue(e, outer, name, strict);
}

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H