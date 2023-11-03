#ifndef ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H
#define ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H

#include <es/types/environment_record.h>

namespace es {

bool HasBinding(Handle<EnvironmentRecord> env_rec, Handle<String> N) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return HasBinding__Declarative(static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N);
  } else {
    ASSERT(env_rec.val()->IsObjectEnv());
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

void CreateAndSetMutableBinding(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return CreateAndSetMutableBinding__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N, D, V, S);
  } else {
    ASSERT(env_rec.val()->IsObjectEnv());
    return CreateAndSetMutableBinding__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N, D, V, S);
  }
}

// 10.2.1.1.2 CreateMutableBinding (N, D)
// 10.2.1.1.3 SetMutableBinding (N,V,S)
void CreateAndSetMutableBinding__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S
) {
  ASSERT(V.val()->IsLanguageType());
  auto entry_fn = [D](HashMapV2::Entry* p) {
    p->can_delete = D;
    p->is_mutable = true;
  };
  auto new_bindings = HashMapV2::Set(Handle<HashMapV2>(env_rec.val()->bindings()), N, V, entry_fn);
  env_rec.val()->SetBindings(new_bindings);
}

bool CreateNotExistsMutableBinding(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S
) {
  ASSERT(V.val()->IsLanguageType());
  ASSERT(env_rec.val()->IsDeclarativeEnv());
  auto entry_fn = [D](HashMapV2::Entry* p) {
    p->can_delete = D;
    p->is_mutable = true;
  };
  bool created;
  auto new_bindings = HashMapV2::Create(Handle<HashMapV2>(env_rec.val()->bindings()), N, V, created, entry_fn);
  env_rec.val()->SetBindings(new_bindings);
  return created;
}

// 10.2.1.2.2 CreateMutableBinding (N, D)
// 10.2.1.2.3 SetMutableBinding (N,V,S)
void CreateAndSetMutableBinding__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S
) {
  ASSERT(V.val()->IsLanguageType());
  StackPropertyDescriptor desc = StackPropertyDescriptor::NewDataDescriptor(V, true, true, D);
  DefineOwnProperty(e, env_rec.val()->bindings(), N, desc, true);
}

void SetMutableBinding(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return SetMutableBinding__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N, V, S);
  } else {
    ASSERT(env_rec.val()->IsObjectEnv());
    return SetMutableBinding__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N, V, S);
  }
}

// 10.2.1.1.3 SetMutableBinding (N,V,S)
void SetMutableBinding__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S
) {
  TEST_LOG("\033[2menter\033[0m SetMutableBinding__Declarative ", N.val()->data(), " to " + V.ToString());
  ASSERT(V.val()->IsLanguageType());
  bool is_mutable;
  auto entry_fn = [&is_mutable, V](HashMapV2::Entry* p) mutable {
    is_mutable = p->is_mutable;
    if (p->is_mutable) {
      p->val = V.val();
    }
  };
  env_rec.val()->bindings()->GetRaw(N, entry_fn);
  if (!is_mutable && S) {
    e = Error::TypeError(u"set value to immutable binding");
  }
}

// 10.2.1.2.3 SetMutableBinding (N,V,S)
void SetMutableBinding__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S
) {
  TEST_LOG("\033[2menter\033[0m SetMutableBinding__Object " + N.ToString() + " to " + V.ToString());
  ASSERT(V.val()->IsLanguageType());
  Put(e, env_rec.val()->bindings(), N, V, S);
}

Handle<JSValue> GetBindingValue(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool S
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return GetBindingValue__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N, S);
  } else {
    ASSERT(env_rec.val()->IsObjectEnv());
    return GetBindingValue__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N, S);
  }
}

// 10.2.1.1.4 GetBindingValue(N,S)
Handle<JSValue> GetBindingValue__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool S
) {
  TEST_LOG("\033[2menter\033[0m GetBindingValue__Declarative " + N.ToString());
  bool is_immutable_undefined = false;
  auto entry_fn = [&is_immutable_undefined] (HashMapV2::Entry* p) mutable {
    is_immutable_undefined = p->val->IsUndefined() && !p->is_mutable;
  };
  // This will always get value.
  JSValue* V = env_rec.val()->bindings()->GetRaw(N, entry_fn);
  ASSERT(V != nullptr);
  if (is_immutable_undefined) {
    if (S) {
      e = Error::ReferenceError(N.val()->data() + u" is not defined");
      return Handle<JSValue>();
    } else {
      if (unlikely(log::Debugger::On()))
        log::PrintSource("GetBindingValue ", N.val()->data(), " undefined");
      return Undefined::Instance();
    }
  }
  TEST_LOG("GetBindingValue ", N.val()->data(), " " + JSValue::ToString(V));
  return Handle<JSValue>(V);
}

// 10.2.1.2.4 GetBindingValue(N,S)
Handle<JSValue> GetBindingValue__Object(
  Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool S
) {
  TEST_LOG("\033[2menter\033[0m GetBindingValue__Object " + N.ToString());
  StackPropertyDescriptor desc = GetProperty(env_rec.val()->bindings(), N);
  if (desc.IsUndefined()) {
    if (S) {
      e = Error::ReferenceError(N.val()->data() + u" is not defined");
      return Handle<JSValue>();
    } else {
      return Undefined::Instance();
    }
  }
  // The other half of Get
  if (desc.IsDataDescriptor()) {
    return desc.Value();
  } else {
    ASSERT(desc.IsAccessorDescriptor());
    Handle<JSValue> getter = desc.Get();
    if (getter.val()->IsUndefined())
      return Undefined::Instance();
    return Call(e, getter, env_rec.val()->bindings());
  }
}

bool DeleteBinding(
  Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N
) {
  if (env_rec.val()->IsDeclarativeEnv()) {
    return DeleteBinding__Declarative(e, static_cast<Handle<DeclarativeEnvironmentRecord>>(env_rec), N);
  } else {
    ASSERT(env_rec.val()->IsObjectEnv());
    return DeleteBinding__Object(e, static_cast<Handle<ObjectEnvironmentRecord>>(env_rec), N);
  }
}

// 10.2.1.1.5 DeleteBinding (N)
bool DeleteBinding__Declarative(
  Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N
) {
  bool can_delete;
  auto entry_fn = [&can_delete](HashMapV2::Entry* p) mutable {
    can_delete = p->can_delete;
  };
  JSValue* V = env_rec.val()->bindings()->GetRaw(N, entry_fn);
  if (V == nullptr) return true;
  if (!can_delete) {
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
    ASSERT(env_rec.val()->IsObjectEnv());
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
// 10.2.1.1.8 InitializeImmutableBinding (N,V)
void CreateAndInitializeImmutableBinding(
  Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V
) {
  auto entry_fn = [](HashMapV2::Entry* p) {
    p->can_delete = false;
    p->is_mutable = false;
  };

  auto new_bindings = HashMapV2::Set(Handle<HashMapV2>(env_rec.val()->bindings()), N, V, entry_fn);
  env_rec.val()->SetBindings(new_bindings);
}

}

#endif  // ES_TYPES_ENVIRONMENT_RECORD_IMPLE_H