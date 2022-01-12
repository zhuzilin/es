#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

class LexicalEnvironment : public JSValue {
 public:
  LexicalEnvironment(JSValue* outer, EnvironmentRecord* env_rec) :
    JSValue(JS_LEX_ENV), env_rec_(env_rec) {
    assert(outer->IsNull() || outer->IsLexicalEnvironment());
    outer_ = outer;
  }

  static LexicalEnvironment* Global() {
    static LexicalEnvironment singleton(
      Null::Instance(), new ObjectEnvironmentRecord(GlobalObject::Instance()));
    return &singleton;
  }

  Reference* GetIdentifierReference(std::u16string name, bool strict) {
    bool exists = env_rec_->HasBinding(name);
    if (exists) {
      return new Reference(env_rec_, name, strict);
    }
    if (outer_->IsNull()) {
      return new Reference(Undefined::Instance(), name, strict);
    }
    LexicalEnvironment* outer = static_cast<LexicalEnvironment*>(outer_);
    return outer->GetIdentifierReference(name, strict);
  }

  static LexicalEnvironment* NewDeclarativeEnvironment(JSValue* lex) {
    DeclarativeEnvironmentRecord* env_rec = new DeclarativeEnvironmentRecord();
    return new LexicalEnvironment(lex, env_rec);
  }

  static LexicalEnvironment* NewObjectEnvironment(JSObject* obj, JSValue* lex) {
    ObjectEnvironmentRecord* env_rec = new ObjectEnvironmentRecord(obj);
    return new LexicalEnvironment(lex, env_rec);
  }

  JSValue* outer() { return outer_; }
  EnvironmentRecord* env_rec() { return env_rec_; }

  std::string ToString() override { return "LexicalEnvironment"; }

 private:
  JSValue* outer_;  // not owned
  EnvironmentRecord* env_rec_;
};

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H