#ifndef ES_ENVIRONMENT_RECORD_H
#define ES_ENVIRONMENT_RECORD_H

#include <string>
#include <string_view>

#include <es/types/base.h>
#include <es/types/object.h>
#include <es/types/property_descriptor.h>

namespace es {

namespace binding {

constexpr size_t kValueOffset = 0;
constexpr size_t kBindingOffset = kValueOffset + sizeof(JSValue);

inline JSValue New(JSValue value, bool can_delete, bool is_mutable) {
  JSValue jsval;
  jsval.handle() = HeapObject::New(sizeof(JSValue));

  SET_JSVALUE(jsval.handle().val(), kValueOffset, value);

  jsval.header_.placeholder_.binding_header_.can_delete_ = can_delete;
  jsval.header_.placeholder_.binding_header_.is_mutable_ = is_mutable;
  jsval.SetType(BINDING);
  return jsval;
}

inline JSValue value(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kValueOffset); };
inline void SetValue(JSValue& jsval, JSValue value) {
  SET_JSVALUE(jsval.handle().val(), kValueOffset, value);
}
inline bool can_delete(JSValue jsval) {
  return jsval.header_.placeholder_.binding_header_.can_delete_;
}
inline bool is_mutable(JSValue jsval) {
  return jsval.header_.placeholder_.binding_header_.is_mutable_;
}

}  // namespace binding

namespace decl_env_rec {

constexpr size_t kBindingsOffset = 0;

inline JSValue New() {
  JSValue jsval;
  jsval.handle() = HeapObject::New(sizeof(JSValue));
  JSValue bindings = hash_map::New();

  SET_JSVALUE(jsval.handle().val(), kBindingsOffset, bindings);
  jsval.SetType(JS_ENV_REC_DECL);
  return jsval;
}

inline JSValue bindings(JSValue jsval) {
  return GET_JSVALUE(jsval.handle().val(), kBindingsOffset);
}
inline void SetBindings(JSValue& jsval, JSValue new_binding) {
  SET_JSVALUE(jsval.handle().val(), kBindingsOffset, new_binding);
}

}  // namespace decl_env_rec

namespace obj_env_rec {

constexpr size_t kBindingsOffset = 0;

inline JSValue New(JSValue obj, bool provide_this = false) {
  JSValue jsval;
  jsval.handle() = HeapObject::New(sizeof(JSValue) + kBoolSize);

  SET_JSVALUE(jsval.handle().val(), kBindingsOffset, obj);
  jsval.header_.placeholder_.provide_this_ = provide_this;
  jsval.SetType(JS_ENV_REC_OBJ);
  return jsval;
}

inline JSValue bindings(JSValue jsval) {
  return GET_JSVALUE(jsval.handle().val(), kBindingsOffset);
}
inline bool provide_this(JSValue jsval) { return jsval.header_.placeholder_.provide_this_; }


}  // namespace obj_env_rec

bool HasBinding(JSValue env_rec, JSValue N);
bool HasBinding__Declarative(JSValue env_rec, JSValue N);
bool HasBinding__Object(JSValue env_rec, JSValue N);

void CreateAndSetMutableBinding(JSValue& e, JSValue env_rec, JSValue N, bool D, JSValue V, bool S);
void CreateAndSetMutableBinding__Declarative(JSValue& e, JSValue env_rec, JSValue N, bool D, JSValue V, bool S);
void CreateAndSetMutableBinding__Object(JSValue& e, JSValue env_rec, JSValue N, bool D, JSValue V, bool S);

void SetMutableBinding(JSValue& e, JSValue env_rec, JSValue N, JSValue V, bool S);
void SetMutableBinding__Declarative(JSValue& e, JSValue env_rec, JSValue N, JSValue V, bool S);
void SetMutableBinding__Object(JSValue& e, JSValue env_rec, JSValue N, JSValue V, bool S);

JSValue GetBindingValue(JSValue& e, JSValue env_rec, JSValue N, bool S);
JSValue GetBindingValue__Declarative(JSValue& e, JSValue env_rec, JSValue N, bool S);
JSValue GetBindingValue__Object(JSValue& e, JSValue env_rec, JSValue N, bool S);

bool DeleteBinding(JSValue& e, JSValue env_rec, JSValue N);
bool DeleteBinding__Declarative(JSValue& e, JSValue env_rec, JSValue N);
bool DeleteBinding__Object(JSValue& e, JSValue env_rec, JSValue N);

JSValue ImplicitThisValue(JSValue env_rec);
JSValue ImplicitThisValue__Declarative(JSValue env_rec);
JSValue ImplicitThisValue__Object(JSValue env_rec);

void CreateAndInitializeImmutableBinding(JSValue env_rec, JSValue N, JSValue V);

}  // namespace es

#endif  // ES_ENVIRONMENT_RECORD_H