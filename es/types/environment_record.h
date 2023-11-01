#ifndef ES_ENVIRONMENT_RECORD_H
#define ES_ENVIRONMENT_RECORD_H

#include <string>
#include <string_view>

#include <es/types/base.h>
#include <es/types/object.h>
#include <es/types/property_descriptor.h>

namespace es {

// EnvironmentRecord is also of type JSValue 
class EnvironmentRecord : public JSValue {
 public:
  template<size_t size>
  static Handle<EnvironmentRecord> New() {
    Handle<JSValue> jsval = HeapObject::New<size + kSizeTSize>();
    SET_VALUE(jsval.val(), kRefCountOffset, 0, size_t);
    return static_cast<Handle<EnvironmentRecord>>(jsval);
  }

  bool IsDeclarativeEnv() { return type() == JS_ENV_REC_DECL; }
  bool IsObjectEnv() { return type() == JS_ENV_REC_OBJ; }

  size_t ref_count() { return READ_VALUE(this, kRefCountOffset, size_t); }
  void AddRefCount() {
    size_t rc = ref_count();
    SET_VALUE(this, kRefCountOffset, rc + 1, size_t);
  }
  size_t ReduceRefCount() {
    ASSERT(ref_count() > 0);
    size_t rc = ref_count();
    SET_VALUE(this, kRefCountOffset, rc - 1, size_t);
    return rc - 1;
  }

 public:
  static constexpr size_t kRefCountOffset = kJSValueOffset;
  static constexpr size_t kEnvironmentRecordOffset = kJSValueOffset + kSizeTSize;
};

class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<DeclarativeEnvironmentRecord> New(size_t num_decls) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New<kPtrSize>();
    auto bindings = HashMapV2::New(num_decls);

    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, bindings, HashMapV2);
    env_rec.val()->SetType(JS_ENV_REC_DECL);
    return Handle<DeclarativeEnvironmentRecord>(env_rec);
  }

  HashMapV2* bindings() { return READ_VALUE(this, kBindingsOffset, HashMapV2*); }
  void SetBindings(Handle<HashMapV2> new_binding) {
    SET_HANDLE_VALUE(this, kBindingsOffset, new_binding, HashMapV2);
  }

 public:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;

  static constexpr size_t kDefaultNumDecls = 8;
};

class ObjectEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<ObjectEnvironmentRecord> New(Handle<JSObject> obj, bool provide_this = false) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New<kPtrSize + kBoolSize>();

    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, obj, JSObject);
    SET_VALUE(env_rec.val(), kProvideThisOffset, provide_this, bool);
    env_rec.val()->SetType(JS_ENV_REC_OBJ);
    return Handle<ObjectEnvironmentRecord>(env_rec);
  }

  Handle<JSObject> bindings() { return READ_HANDLE_VALUE(this, kBindingsOffset, JSObject); }
  bool provide_this() { return READ_VALUE(this, kProvideThisOffset, bool); }

 public:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
  static constexpr size_t kProvideThisOffset = kBindingsOffset + kPtrSize;
};

bool HasBinding(Handle<EnvironmentRecord> env_rec, Handle<String> N);
bool HasBinding__Declarative(Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N);
bool HasBinding__Object(Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N);

void CreateAndSetMutableBinding(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S);
void CreateAndSetMutableBinding__Declarative(Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S);
void CreateAndSetMutableBinding__Object(Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool D, Handle<JSValue> V, bool S);

void SetMutableBinding(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S);
void SetMutableBinding__Declarative(Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S);
void SetMutableBinding__Object(Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V, bool S);

Handle<JSValue> GetBindingValue(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool S);
Handle<JSValue> GetBindingValue__Declarative(Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool S);
Handle<JSValue> GetBindingValue__Object(Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool S);

bool DeleteBinding(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N);
bool DeleteBinding__Declarative(Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N);
bool DeleteBinding__Object(Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N);

Handle<JSValue> ImplicitThisValue(Handle<EnvironmentRecord> env_rec);
Handle<JSValue> ImplicitThisValue__Declarative(Handle<DeclarativeEnvironmentRecord> env_rec);
Handle<JSValue> ImplicitThisValue__Object(Handle<ObjectEnvironmentRecord> env_rec);

void CreateAndInitializeImmutableBinding(Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V);

}  // namespace es

#endif  // ES_ENVIRONMENT_RECORD_H