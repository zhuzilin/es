#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

class LexicalEnvironment : public JSValue {
 public:
  static Handle<LexicalEnvironment> New(Handle<LexicalEnvironment> outer, Handle<EnvironmentRecord> env_rec) {
    Handle<JSValue> jsval = HeapObject::New(2 * kPtrSize);

    SET_HANDLE_VALUE(jsval.val(), kOuterOffset, outer, LexicalEnvironment);
    SET_HANDLE_VALUE(jsval.val(), kEnvRecOffset, env_rec, EnvironmentRecord);

    jsval.val()->SetType(JS_LEX_ENV);
    return Handle<LexicalEnvironment>(jsval);
  }

  Handle<LexicalEnvironment> outer() { return READ_HANDLE_VALUE(this, kOuterOffset, LexicalEnvironment); }
  Handle<EnvironmentRecord> env_rec() { return READ_HANDLE_VALUE(this, kEnvRecOffset, EnvironmentRecord); }

  static Handle<LexicalEnvironment> Global() {
    static Handle<LexicalEnvironment> singleton = LexicalEnvironment::New(
      Handle<LexicalEnvironment>(), ObjectEnvironmentRecord::New(GlobalObject::Instance()));
    return singleton;
  }

 public:
  static constexpr size_t kOuterOffset = kJSValueOffset;
  static constexpr size_t kEnvRecOffset = kOuterOffset + kPtrSize;
};

Handle<LexicalEnvironment> NewDeclarativeEnvironment(Handle<LexicalEnvironment> lex) {
  Handle<DeclarativeEnvironmentRecord> env_rec = DeclarativeEnvironmentRecord::New();
  return LexicalEnvironment::New(lex, env_rec);
}

Handle<LexicalEnvironment> NewObjectEnvironment(Handle<JSObject> obj, Handle<LexicalEnvironment> lex, bool provide_this = false) {
  Handle<ObjectEnvironmentRecord> env_rec = ObjectEnvironmentRecord::New(obj, provide_this);
  return LexicalEnvironment::New(lex, env_rec);
}

Handle<Reference> GetIdentifierReference(Handle<LexicalEnvironment> lex, Handle<String> name, bool strict) {
  auto env_rec = lex.val()->env_rec();
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return Reference::New(env_rec, name, strict);
  }
  auto outer = lex.val()->outer();
  if (outer.IsNullptr()) {
    return Reference::New(Undefined::Instance(), name, strict);
  }
  return GetIdentifierReference(outer, name, strict);
}

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