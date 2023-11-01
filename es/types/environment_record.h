#ifndef ES_ENVIRONMENT_RECORD_H
#define ES_ENVIRONMENT_RECORD_H

#include <string>
#include <string_view>

#include <es/types/base.h>
#include <es/types/object.h>
#include <es/types/property_descriptor.h>

namespace es {

// EnvironmentRecord is a merge of EnvironmentRecord and LexicalEnv
class EnvironmentRecord : public JSValue {
 public:
  template<size_t size>
  static Handle<EnvironmentRecord> New(Handle<JSValue> outer) {
    Handle<JSValue> jsval = HeapObject::New<size + kEnvironmentRecordOffset - kJSValueOffset>();
    SET_VALUE(jsval.val(), kRefCountOffset, 0, size_t);
    SET_HANDLE_VALUE(jsval.val(), kOuterOffset, outer, JSValue);

    return static_cast<Handle<EnvironmentRecord>>(jsval);
  }

  size_t ref_count() { return READ_VALUE(this, kRefCountOffset, size_t); }
  void AddRefCount() {
    ASSERT(!env_rec().IsNullptr());
    size_t old_rc = ref_count();
    SET_VALUE(this, kRefCountOffset, old_rc + 1, size_t);
    // First use, add 1 to outer env_rec.
    if (old_rc == 0) {
      if (unlikely(outer().IsNullptr())) {
        return;
      }
      outer().val()->AddRefCount();
    }
  }

  // Reduce the ref_count of outer lexical env.
  void ReduceRefCount() {
    ASSERT(ref_count() > 0);
    size_t rc = ref_count() - 1;
    SET_VALUE(this, kRefCountOffset, rc, size_t);
    // No longer use, reduce 1 to outer env_rec
    if (rc == 0) {
      if (!outer().IsNullptr()) {
        outer().val()->ReduceRefCount();
      }
    }
  }

  void SetOuter(Handle<EnvironmentRecord> outer) {
    SET_HANDLE_VALUE(this, kOuterOffset, outer, EnvironmentRecord);
  }
  Handle<EnvironmentRecord> outer() { return READ_HANDLE_VALUE(this, kOuterOffset, EnvironmentRecord); }

  static Handle<EnvironmentRecord> Global();

  bool IsDeclarativeEnv() { return type() == JS_ENV_REC_DECL; }
  bool IsObjectEnv() { return type() == JS_ENV_REC_OBJ; }

 public:
  static constexpr size_t kRefCountOffset = kJSValueOffset;
  static constexpr size_t kOuterOffset = kRefCountOffset + kSizeTSize;
  static constexpr size_t kEnvironmentRecordOffset = kOuterOffset + kPtrSize;
};

class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<DeclarativeEnvironmentRecord> New(Handle<JSValue> outer, size_t num_decls) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New<kPtrSize>(outer);
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
  static Handle<ObjectEnvironmentRecord> New(Handle<JSValue> outer, Handle<JSObject> obj, bool provide_this = false) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New<kPtrSize + kBoolSize>(outer);

    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, obj, JSObject);
    SET_VALUE(env_rec.val(), kProvideThisOffset, provide_this, bool);
    env_rec.val()->SetType(JS_ENV_REC_OBJ);
    return Handle<ObjectEnvironmentRecord>(env_rec);
  }

  Handle<JSObject> bindings() { return READ_HANDLE_VALUE(this, kBindingsOffset, JSObject); }
  bool provide_this() { return READ_VALUE(this, kProvideThisOffset, bool); }

  static Handle<EnvironmentRecord> Global();

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