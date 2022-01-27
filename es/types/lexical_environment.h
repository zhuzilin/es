#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

class LexicalEnvironment : public JSValue {
 public:
  static LexicalEnvironment* New(LexicalEnvironment* outer, EnvironmentRecord* env_rec) {
    JSValue* jsval = JSValue::New(JS_LEX_ENV, 2 * kPtrSize);
    SET_VALUE(jsval, kOuterOffset, outer, LexicalEnvironment*);
    SET_VALUE(jsval, kEnvRecOffset, env_rec, EnvironmentRecord*);
    return new (jsval) LexicalEnvironment();
  }

  std::vector<void*> Pointers() override {
    return {HEAP_PTR(kOuterOffset), HEAP_PTR(kEnvRecOffset)};
  }

  LexicalEnvironment* outer() { return READ_VALUE(this, kOuterOffset, LexicalEnvironment*); }
  EnvironmentRecord* env_rec() { return READ_VALUE(this, kEnvRecOffset, EnvironmentRecord*); }

  static LexicalEnvironment* Global() {
    static LexicalEnvironment* singleton = LexicalEnvironment::New(
      nullptr, ObjectEnvironmentRecord::New(GlobalObject::Instance()));
    return singleton;
  }

  Reference* GetIdentifierReference(String* name, bool strict) {
    bool exists = env_rec()->HasBinding(name);
    if (exists) {
      return Reference::New(env_rec(), name, strict);
    }
    if (outer() == nullptr) {
      return Reference::New(Undefined::Instance(), name, strict);
    }
    return outer()->GetIdentifierReference(name, strict);
  }

  static LexicalEnvironment* NewDeclarativeEnvironment(LexicalEnvironment* lex) {
    DeclarativeEnvironmentRecord* env_rec = DeclarativeEnvironmentRecord::New();
    return LexicalEnvironment::New(lex, env_rec);
  }

  static LexicalEnvironment* NewObjectEnvironment(JSObject* obj, LexicalEnvironment* lex, bool provide_this = false) {
    ObjectEnvironmentRecord* env_rec = ObjectEnvironmentRecord::New(obj, provide_this);
    return LexicalEnvironment::New(lex, env_rec);
  }

  std::string ToString() override { return "LexicalEnvironment"; }

 private:
  static constexpr size_t kOuterOffset = kJSValueOffset;
  static constexpr size_t kEnvRecOffset = kOuterOffset + kPtrSize;
};

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H