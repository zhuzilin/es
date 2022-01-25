#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

class LexicalEnvironment {
 public:
  LexicalEnvironment(LexicalEnvironment* outer, EnvironmentRecord* env_rec) :
    outer_(outer), env_rec_(env_rec) {}

  static LexicalEnvironment* Global() {
    static LexicalEnvironment* singleton = new LexicalEnvironment(
      nullptr, new ObjectEnvironmentRecord(GlobalObject::Instance()));
    return singleton;
  }

  Reference* GetIdentifierReference(std::u16string name, bool strict) {
    bool exists = env_rec_->HasBinding(name);
    if (exists) {
      return new Reference(env_rec_, name, strict);
    }
    if (outer_ == nullptr) {
      return new Reference(Undefined::Instance(), name, strict);
    }
    LexicalEnvironment* outer = static_cast<LexicalEnvironment*>(outer_);
    return outer->GetIdentifierReference(name, strict);
  }

  static LexicalEnvironment* NewDeclarativeEnvironment(LexicalEnvironment* lex) {
    DeclarativeEnvironmentRecord* env_rec = new DeclarativeEnvironmentRecord();
    return new LexicalEnvironment(lex, env_rec);
  }

  static LexicalEnvironment* NewObjectEnvironment(JSObject* obj, LexicalEnvironment* lex, bool provide_this = false) {
    ObjectEnvironmentRecord* env_rec = new ObjectEnvironmentRecord(obj, provide_this);
    return new LexicalEnvironment(lex, env_rec);
  }

  LexicalEnvironment* outer() { return outer_; }
  EnvironmentRecord* env_rec() { return env_rec_; }

  std::string ToString() { return "LexicalEnvironment"; }

  std::vector<void*> Pointers() {
    std::vector<void*> pointers;
    std::cout << "add pointer: outer " << outer_ << std::endl;
    pointers.emplace_back(&outer_);
    pointers.emplace_back(&env_rec_);
    return pointers;
  }

 private:
  LexicalEnvironment* outer_;  // not owned
  EnvironmentRecord* env_rec_;
};

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H