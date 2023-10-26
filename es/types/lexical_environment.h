#ifndef ES_LEXICAL_ENVIRONMENT_H
#define ES_LEXICAL_ENVIRONMENT_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/reference.h>

namespace es {

namespace lexical_env {

constexpr size_t kOuterOffset = 0;
constexpr size_t kEnvRecOffset = kOuterOffset + sizeof(JSValue);

inline JSValue New(JSValue outer, JSValue env_rec) {
  JSValue jsval;
  jsval.handle() = HeapObject::New(2 * sizeof(JSValue));

  SET_JSVALUE(jsval.handle().val(), kOuterOffset, outer);
  SET_JSVALUE(jsval.handle().val(), kEnvRecOffset, env_rec);

  jsval.SetType(JS_LEX_ENV);
  return jsval;
}

inline JSValue outer(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kOuterOffset); }
inline JSValue env_rec(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kEnvRecOffset); }

inline JSValue Global() {
  static JSValue singleton = lexical_env::New(
    null::New(), obj_env_rec::New(global_object::Instance()));
  return singleton;
}

}  // namespace lexical_env

inline JSValue NewDeclarativeEnvironment(JSValue lex) {
  JSValue env_rec = decl_env_rec::New();
  return lexical_env::New(lex, env_rec);
}

inline JSValue NewObjectEnvironment(JSValue obj, JSValue lex, bool provide_this = false) {
  JSValue env_rec = obj_env_rec::New(obj, provide_this);
  return lexical_env::New(lex, env_rec);
}

inline JSValue GetIdentifierReference(JSValue lex, JSValue name, bool strict) {
  ASSERT(lex.IsLexicalEnvironment());
  ASSERT(name.IsString());
  JSValue env_rec = lexical_env::env_rec(lex);
  bool exists = HasBinding(env_rec, name);
  if (exists) {
    return reference::New(env_rec, name, strict);
  }
  auto outer = lexical_env::outer(lex);
  if (outer.IsNull()) {
    return reference::New(undefined::New(), name, strict);
  }
  return GetIdentifierReference(outer, name, strict);
}

}  // namespace es

#endif  // ES_LEXICAL_ENVIRONMENT_H