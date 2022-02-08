#ifndef ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H
#define ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H

#include <es/types/environment_record.h>

namespace es {

bool HasBinding(Handle<EnvironmentRecord> env_rec, Handle<String> N) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return HasBinding__Declarative(static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N);
  } else {
    assert(env_rec.val()->IsObjectEnv());
    return HasBinding__Object(static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N);
  }
}

// 10.2.1.1.1 HasBinding(N)
bool HasBinding__Declarative(Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N) {
  return env_rec.val()->bindings()->GetRaw(N) != nullptr;
}

// 10.2.1.2.1 HasBinding(N)
bool HasBinding__Object(Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N) {
  return HasProperty(env_rec.val()->bindings(), N);
}

void CreateMutableBinding(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool D
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return CreateMutableBinding__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N, D);
  } else {
    assert(env_rec.val()->IsObjectEnv());
    return CreateMutableBinding__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N, D);
  }
}

// 10.2.1.1.2 CreateMutableBinding (N, D)
void CreateMutableBinding__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool D
) {
  assert(!HasBinding(env_rec, N));
  Handle<DeclarativeEnvironmentRecord::Binding> b = DeclarativeEnvironmentRecord::Binding::New(
    Undefined::Instance(), D, true);
  auto new_bindings = HashMap<DeclarativeEnvironmentRecord::Binding>::Set(
    Handle<HashMap<DeclarativeEnvironmentRecord::Binding>>(env_rec.val()->bindings()), N, b);
  env_rec.val()->SetBindings(new_bindings);
}

// 10.2.1.2.2 CreateMutableBinding (N, D)
void CreateMutableBinding__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool D
) {
  assert(!HasBinding(env_rec, N));
  Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
  desc.val()->SetDataDescriptor(Undefined::Instance(), true, true, D);
  DefineOwnProperty(e, env_rec.val()->bindings(), N, desc, true);
}

void SetMutableBinding(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return SetMutableBinding__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N, V, S);
  } else {
    assert(env_rec.val()->IsObjectEnv());
    return SetMutableBinding__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N, V, S);
  }
}

// 10.2.1.1.3 SetMutableBinding (N,V,S)
void SetMutableBinding__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S
) {
  if (log::Debugger::On())
    log::PrintSource("enter SetMutableBinding__Declarative ", N.val()->data(), " to " + V.ToString());
  assert(V.val()->IsLanguageType());
  assert(HasBinding(env_rec, N));
  // NOTE(zhuzilin) If we do note b = bindings_[N] and change b.value,
  // the value stored in bindings_ won't change.
  if (env_rec.val()->bindings()->GetRaw(N)->is_mutable()) {
    env_rec.val()->bindings()->GetRaw(N)->SetValue(V);
  } else if (S) {
    e = Error::TypeError();
  }
}

// 10.2.1.2.3 SetMutableBinding (N,V,S)
void SetMutableBinding__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S
) {
  if (log::Debugger::On())
    log::PrintSource("enter SetMutableBinding__Object " + N.ToString() + " to " + V.ToString());
  assert(V.val()->IsLanguageType());
  Put(e, env_rec.val()->bindings(), N, V, S);
}

Handle<JSValue> GetBindingValue(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool S
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return GetBindingValue__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N, S);
  } else {
    assert(env_rec.val()->IsObjectEnv());
    return GetBindingValue__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N, S);
  }
}

// 10.2.1.1.4 GetBindingValue(N,S)
Handle<JSValue> GetBindingValue__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool S
) {
  if (log::Debugger::On())
    log::PrintSource("enter GetBindingValue__Declarative " + N.ToString());
  assert(HasBinding(env_rec, N));
  DeclarativeEnvironmentRecord::Binding* b = env_rec.val()->bindings()->GetRaw(N);
  if (b->value().val()->IsUndefined() && !b->is_mutable()) {
    if (S) {
      e = Error::ReferenceError(N.val()->data() + u" is not defined");
      return Handle<JSValue>();
    } else {
      if (log::Debugger::On())
        log::PrintSource("GetBindingValue ", N.val()->data(), " undefined");
      return Undefined::Instance();
    }
  }
  if (log::Debugger::On())
    log::PrintSource("GetBindingValue ", N.val()->data(), " " + b->value().ToString());
  return b->value();
}

// 10.2.1.2.4 GetBindingValue(N,S)
Handle<JSValue> GetBindingValue__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool S
) {
  if (log::Debugger::On())
    log::PrintSource("enter GetBindingValue__Object " + N.ToString());
  bool value = HasBinding(env_rec, N);
  if (!value) {
    if (S) {
      e = Error::ReferenceError(N.val()->data() + u" is not defined");
      return Handle<JSValue>();
    } else {
      return Undefined::Instance();
    }
  }
  return Get(e, env_rec.val()->bindings(), N);
}

bool DeleteBinding(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return DeleteBinding__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N);
  } else {
    assert(env_rec.val()->IsObjectEnv());
    return DeleteBinding__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N);
  }
}

// 10.2.1.1.5 DeleteBinding (N)
bool DeleteBinding__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N
) {
  if (!HasBinding(env_rec, N)) return true;
  if (!env_rec.val()->bindings()->GetRaw(N)->can_delete()) {
    return false;
  }
  env_rec.val()->bindings()->Delete(N);
  return true;
}

// 10.2.1.2.5 DeleteBinding (N)
bool DeleteBinding__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N
) {
  return Delete(e, env_rec.val()->bindings(), N, false);
}

Handle<JSValue> ImplicitThisValue(Handle<EnvironmentRecord> env_rec) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return ImplicitThisValue__Declarative(static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec));
  } else {
    assert(env_rec.val()->IsObjectEnv());
    return ImplicitThisValue__Object(static_cast<Handle<ObjectEnvironmentRecord>>(env_rec));
  }
}

// 10.2.1.1.6 ImplicitThisValue()
Handle<JSValue> ImplicitThisValue__Declarative(Handle<DeclarativeEnvironmentRecord> env_rec) {
  return Undefined::Instance();
}

// 10.2.1.2.6 ImplicitThisValue()
Handle<JSValue> ImplicitThisValue__Object(Handle<ObjectEnvironmentRecord> env_rec) {
  if (env_rec.val()->provide_this()) {
    return env_rec.val()->bindings();
  }
  return Undefined::Instance();
}

// 10.2.1.1.7 CreateImmutableBinding (N)
void CreateImmutableBinding(
  Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N
) {
  assert(!HasBinding(env_rec, N));
  Handle<DeclarativeEnvironmentRecord::Binding> b = DeclarativeEnvironmentRecord::Binding::New(
    Undefined::Instance(), false, false);
  auto new_bindings = HashMap<DeclarativeEnvironmentRecord::Binding>::Set(
    Handle<HashMap<DeclarativeEnvironmentRecord::Binding>>(env_rec.val()->bindings()), N, b);
  env_rec.val()->SetBindings(new_bindings);
}

// 10.2.1.1.8 InitializeImmutableBinding (N,V)
void InitializeImmutableBinding(
  Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V
) {
  assert(HasBinding(env_rec, N));
  assert(!env_rec.val()->bindings()->GetRaw(N)->is_mutable() &&
         env_rec.val()->bindings()->GetRaw(N)->value().val()->IsUndefined());
  env_rec.val()->bindings()->GetRaw(N)->SetValue(V);
}

}

#endif  // ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H