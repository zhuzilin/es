#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

class LexicalEnvironment : public JSValue {
 public:
  static Handle<LexicalEnvironment> New(Handle<LexicalEnvironment> outer, Handle<EnvironmentRecord> env_rec) {
    Handle<JSValue> jsval = JSValue::New(JS_LEX_ENV, 2 * kPtrSize);
    SET_HANDLE_VALUE(jsval.val(), kOuterOffset, outer, LexicalEnvironment);
    SET_HANDLE_VALUE(jsval.val(), kEnvRecOffset, env_rec, EnvironmentRecord);
    return Handle<LexicalEnvironment>(new (jsval.val()) LexicalEnvironment());
  }

  std::vector<HeapObject**> Pointers() override {
    return {HEAP_PTR(kOuterOffset), HEAP_PTR(kEnvRecOffset)};
  }

  LexicalEnvironment* outer() { return READ_VALUE(this, kOuterOffset, LexicalEnvironment*); }
  EnvironmentRecord* env_rec() { return READ_VALUE(this, kEnvRecOffset, EnvironmentRecord*); }

  static Handle<LexicalEnvironment> Global() {
    static Handle<LexicalEnvironment> singleton = LexicalEnvironment::New(
      Handle<LexicalEnvironment>(), ObjectEnvironmentRecord::New(GlobalObject::Instance()));
    return singleton;
  }

  Handle<Reference> GetIdentifierReference(Handle<String> name, bool strict) {
    bool exists = env_rec()->HasBinding(name);
    if (exists) {
      return Reference::New(Handle<EnvironmentRecord>(env_rec()), name, strict);
    }
    if (outer() == nullptr) {
      return Reference::New(Undefined::Instance(), name, strict);
    }
    return outer()->GetIdentifierReference(name, strict);
  }

  static Handle<LexicalEnvironment> NewDeclarativeEnvironment(Handle<LexicalEnvironment> lex) {
    Handle<DeclarativeEnvironmentRecord> env_rec = DeclarativeEnvironmentRecord::New();
    return LexicalEnvironment::New(lex, env_rec);
  }

  static Handle<LexicalEnvironment> NewObjectEnvironment(Handle<JSObject> obj, Handle<LexicalEnvironment> lex, bool provide_this = false) {
    Handle<ObjectEnvironmentRecord> env_rec = ObjectEnvironmentRecord::New(obj, provide_this);
    return LexicalEnvironment::New(lex, env_rec);
  }

  std::string ToString() override { return "LexicalEnvironment"; }

 private:
  static constexpr size_t kOuterOffset = kJSValueOffset;
  static constexpr size_t kEnvRecOffset = kOuterOffset + kPtrSize;
};

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H