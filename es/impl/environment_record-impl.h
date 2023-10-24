#ifndef ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H
#define ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H

#include <es/types/environment_record.h>

namespace es {

bool HasBinding(JSValue env_rec, JSValue N) {
  if (env_rec.IsDeclarativeEnv()) {
    return HasBinding__Declarative(env_rec, N);
  } else {
    ASSERT(env_rec.IsObjectEnv());
    return HasBinding__Object(env_rec, N);
  }
}

// 10.2.1.1.1 HasBinding(N)
bool HasBinding__Declarative(JSValue env_rec, JSValue N) {
  return !hash_map::Get(decl_env_rec::bindings(env_rec), N).IsNull();
}

// 10.2.1.2.1 HasBinding(N)
bool HasBinding__Object(JSValue env_rec, JSValue N) {
  return HasProperty(obj_env_rec::bindings(env_rec), N);
}

void CreateAndSetMutableBinding(
  JSValue& e, JSValue env_rec, JSValue N, bool D, JSValue V, bool S
) {
  if (env_rec.IsDeclarativeEnv()) {
    return CreateAndSetMutableBinding__Declarative(e, env_rec, N, D, V, S);
  } else {
    ASSERT(env_rec.IsObjectEnv());
    return CreateAndSetMutableBinding__Object(e, env_rec, N, D, V, S);
  }
}

// 10.2.1.1.2 CreateMutableBinding (N, D)
// 10.2.1.1.3 SetMutableBinding (N,V,S)
void CreateAndSetMutableBinding__Declarative(
  JSValue& e, JSValue env_rec, JSValue N, bool D, JSValue V, bool S
) {
  ASSERT(V.IsLanguageType());
  JSValue b = binding::New(undefined::New(), D, true);
  binding::SetValue(b, V);
  JSValue new_bindings = hash_map::Set(decl_env_rec::bindings(env_rec), N, b);
  decl_env_rec::SetBindings(env_rec, new_bindings);
}

// 10.2.1.2.2 CreateMutableBinding (N, D)
// 10.2.1.2.3 SetMutableBinding (N,V,S)
void CreateAndSetMutableBinding__Object(
  JSValue& e, JSValue env_rec, JSValue N, bool D, JSValue V, bool S
) {
  ASSERT(V.IsLanguageType());
  JSValue desc = property_descriptor::New();
  property_descriptor::SetDataDescriptor(desc, V, true, true, D);
  DefineOwnProperty(e, obj_env_rec::bindings(env_rec), N, desc, true);
}

void SetMutableBinding(
  JSValue& e, JSValue env_rec, JSValue N, JSValue V, bool S
) {
  if (env_rec.IsDeclarativeEnv()) {
    return SetMutableBinding__Declarative(e, env_rec, N, V, S);
  } else {
    ASSERT(env_rec.IsObjectEnv());
    return SetMutableBinding__Object(e, env_rec, N, V, S);
  }
}

// 10.2.1.1.3 SetMutableBinding (N,V,S)
void SetMutableBinding__Declarative(
  JSValue& e, JSValue env_rec, JSValue N, JSValue V, bool S
) {
  TEST_LOG("enter SetMutableBinding__Declarative ", string::data(N), " to " + JSValue::ToString(V));
  ASSERT(V.IsLanguageType());
  // NOTE(zhuzilin) If we do note b = bindings_[N] and change b.value,
  // the value stored in bindings_ won't change.
  JSValue binding = hash_map::Get(decl_env_rec::bindings(env_rec), N);
  if (binding::is_mutable(binding)) {
    binding::SetValue(binding, V);
  } else if (S) {
    e = error::TypeError(u"set value to immutable binding");
  }
}

// 10.2.1.2.3 SetMutableBinding (N,V,S)
void SetMutableBinding__Object(
  JSValue& e, JSValue env_rec, JSValue N, JSValue V, bool S
) {
  TEST_LOG("enter SetMutableBinding__Object " + JSValue::ToString(N) + " to " + JSValue::ToString(V));
  ASSERT(V.IsLanguageType());
  Put(e, obj_env_rec::bindings(env_rec), N, V, S);
}

JSValue GetBindingValue(
  JSValue& e, JSValue env_rec, JSValue N, bool S
) {
  if (env_rec.IsDeclarativeEnv()) {
    return GetBindingValue__Declarative(e, env_rec, N, S);
  } else {
    ASSERT(env_rec.IsObjectEnv());
    return GetBindingValue__Object(e, env_rec, N, S);
  }
}

// 10.2.1.1.4 GetBindingValue(N,S)
JSValue GetBindingValue__Declarative(
  JSValue& e, JSValue env_rec, JSValue N, bool S
) {
  TEST_LOG("enter GetBindingValue__Declarative " + JSValue::ToString(N));
  JSValue b = hash_map::Get(decl_env_rec::bindings(env_rec), N);
  ASSERT(!b.IsNull());
  if (b.IsUndefined() && !binding::is_mutable(b)) {
    if (S) {
      e = error::ReferenceError(string::data(N) + u" is not defined");
      return JSValue();
    } else {
      if (unlikely(log::Debugger::On()))
        log::PrintSource("GetBindingValue ", string::data(N), " undefined");
      return undefined::New();
    }
  }
  ASSERT(b.type() == BINDING);
  JSValue val = binding::value(b);
  TEST_LOG("GetBindingValue ", string::data(N), " " + JSValue::ToString(val));
  return val;
}

// 10.2.1.2.4 GetBindingValue(N,S)
JSValue GetBindingValue__Object(
  JSValue& e, JSValue env_rec, JSValue N, bool S
) {
  TEST_LOG("enter GetBindingValue__Object " + JSValue::ToString(N));
  bool value = HasBinding(env_rec, N);
  if (!value) {
    if (S) {
      e = error::ReferenceError(string::data(N) + u" is not defined");
      return JSValue();
    } else {
      return undefined::New();
    }
  }
  return Get(e, obj_env_rec::bindings(env_rec), N);
}

bool DeleteBinding(
  JSValue& e, JSValue env_rec, JSValue N
) {
  if (env_rec.IsDeclarativeEnv()) {
    return DeleteBinding__Declarative(e, env_rec, N);
  } else {
    ASSERT(env_rec.IsObjectEnv());
    return DeleteBinding__Object(e, env_rec, N);
  }
}

// 10.2.1.1.5 DeleteBinding (N)
bool DeleteBinding__Declarative(
  JSValue& e, JSValue env_rec, JSValue N
) {
  JSValue b = hash_map::Get(decl_env_rec::bindings(env_rec), N);
  if (b.IsNull()) return true;
  if (!binding::can_delete(b)) {
    return false;
  }
  hash_map::Delete(decl_env_rec::bindings(env_rec), N);
  return true;
}

// 10.2.1.2.5 DeleteBinding (N)
bool DeleteBinding__Object(
  JSValue& e, JSValue env_rec, JSValue N
) {
  return Delete(e, obj_env_rec::bindings(env_rec), N, false);
}

JSValue ImplicitThisValue(JSValue env_rec) {
  if (env_rec.IsDeclarativeEnv()) {
    return ImplicitThisValue__Declarative(env_rec);
  } else {
    ASSERT(env_rec.IsObjectEnv());
    return ImplicitThisValue__Object(env_rec);
  }
}

// 10.2.1.1.6 ImplicitThisValue()
JSValue ImplicitThisValue__Declarative(JSValue env_rec) {
  return undefined::New();
}

// 10.2.1.2.6 ImplicitThisValue()
JSValue ImplicitThisValue__Object(JSValue env_rec) {
  if (obj_env_rec::provide_this(env_rec)) {
    return obj_env_rec::bindings(env_rec);
  }
  return undefined::New();
}

// 10.2.1.1.7 CreateImmutableBinding (N)
// 10.2.1.1.8 InitializeImmutableBinding (N,V)
void CreateAndInitializeImmutableBinding(
  JSValue env_rec, JSValue N, JSValue V
) {
  JSValue b = binding::New(
    undefined::New(), false, false);
  binding::SetValue(b, V);
  auto new_bindings = hash_map::Set(decl_env_rec::bindings(env_rec), N, b);
  decl_env_rec::SetBindings(env_rec, new_bindings);
}

}

#endif  // ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H