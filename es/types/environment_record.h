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
  static Handle<EnvironmentRecord> New(size_t size) {
    Handle<JSValue> jsval = JSValue::New(size);
    return static_cast<Handle<EnvironmentRecord>>(jsval);
  }

  bool IsDeclarativeEnv() { return type() == JS_ENV_REC_DECL; }
  bool IsObjectEnv() { return type() == JS_ENV_REC_OBJ; }

 public:
  static constexpr size_t kEnvironmentRecordOffset = kJSValueOffset;
};

class Binding : public HeapObject {
 public:
  static Handle<Binding> New(Handle<JSValue> value, bool can_delete, bool is_mutable) {
#ifdef GC_DEBUG
    if (log::Debugger::On())
      std::cout << "Binding::New" << "\n";
#endif
    Handle<HeapObject> heap_obj = HeapObject::New(kBindingOffset - kHeapObjectOffset);

    SET_HANDLE_VALUE(heap_obj.val(), kValueOffset, value, JSValue);
    SET_VALUE(heap_obj.val(), kCanDeleteOffset, can_delete, bool);
    SET_VALUE(heap_obj.val(), kIsMutableOffset, is_mutable, bool);

    heap_obj.val()->SetType(BINDING);
    return Handle<Binding>(heap_obj);
  }

  Handle<JSValue> value() { return READ_HANDLE_VALUE(this, kValueOffset, JSValue); };
  void SetValue(Handle<JSValue> value) { SET_HANDLE_VALUE(this, kValueOffset, value, JSValue); }
  bool can_delete() { return READ_VALUE(this, kCanDeleteOffset, bool); }
  bool is_mutable() { return READ_VALUE(this, kIsMutableOffset, bool); }

 public:
  static constexpr size_t kValueOffset = kHeapObjectOffset;
  static constexpr size_t kCanDeleteOffset = kValueOffset + kPtrSize;
  static constexpr size_t kIsMutableOffset = kCanDeleteOffset + kBoolSize;
  static constexpr size_t kBindingOffset = kIsMutableOffset + kBoolSize;
};


class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<DeclarativeEnvironmentRecord> New() {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New(kPtrSize);
    auto bindings = HashMap::New();

    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, bindings, HashMap);
    env_rec.val()->SetType(JS_ENV_REC_DECL);
    return Handle<DeclarativeEnvironmentRecord>(env_rec);
  }

  HashMap* bindings() { return READ_VALUE(this, kBindingsOffset, HashMap*); }
  void SetBindings(Handle<HashMap> new_binding) {
    SET_HANDLE_VALUE(this, kBindingsOffset, new_binding, HashMap);
  }

 public:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
};

class ObjectEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<ObjectEnvironmentRecord> New(Handle<JSObject> obj, bool provide_this = false) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New(kPtrSize + kBoolSize);

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

void CreateMutableBinding(Handle<Error>& e, Handle<EnvironmentRecord> env_rec, Handle<String> N, bool D);
void CreateMutableBinding__Declarative(Handle<Error>& e, Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, bool D);
void CreateMutableBinding__Object(Handle<Error>& e, Handle<ObjectEnvironmentRecord> env_rec, Handle<String> N, bool D);

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

void CreateImmutableBinding(Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N);
void InitializeImmutableBinding(Handle<DeclarativeEnvironmentRecord> env_rec, Handle<String> N, Handle<JSValue> V);

}  // namespace es

#endif  // ES_ENVIRONMENT_RECORD_H