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
  enum EnvRecType {
    ENV_DECL,
    ENV_OBJ,
  };

  static Handle<EnvironmentRecord> New(EnvRecType type, size_t size) {
    Handle<JSValue> jsval = JSValue::New(JS_ENV_REC, size + kIntSize);

    SET_VALUE(jsval.val(), kEnvRecTypeOffset, type, EnvRecType);

    return static_cast<Handle<EnvironmentRecord>>(jsval);
  }

  EnvRecType env_rec_type() { return READ_VALUE(this, kEnvRecTypeOffset, EnvRecType); }
  bool IsDeclarativeEnv() { return env_rec_type() == ENV_DECL; }
  bool IsObjectEnv() { return env_rec_type() == ENV_OBJ; }

 protected:
  static constexpr size_t kEnvRecTypeOffset = kJSValueOffset;
  static constexpr size_t kEnvironmentRecordOffset = kEnvRecTypeOffset + kIntSize;
};

class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  class Binding : public HeapObject {
   public:
    static Handle<Binding> New(Handle<JSValue> value, bool can_delete, bool is_mutable) {
#ifdef GC_DEBUG
      if (log::Debugger::On())
        std::cout << "Binding::New" << std::endl;
#endif
      Handle<HeapObject> heap_obj = HeapObject::New(kBindingOffset - kHeapObjectOffset);

      SET_HANDLE_VALUE(heap_obj.val(), kValueOffset, value, JSValue);
      SET_VALUE(heap_obj.val(), kCanDeleteOffset, can_delete, bool);
      SET_VALUE(heap_obj.val(), kIsMutableOffset, is_mutable, bool);

      new (heap_obj.val()) Binding();
      return Handle<Binding>(heap_obj);
    }

    Handle<JSValue> value() { return READ_HANDLE_VALUE(this, kValueOffset, JSValue); };
    void SetValue(Handle<JSValue> value) { SET_HANDLE_VALUE(this, kValueOffset, value, JSValue); }
    bool can_delete() { return READ_VALUE(this, kCanDeleteOffset, bool); }
    bool is_mutable() { return READ_VALUE(this, kIsMutableOffset, bool); }

    std::vector<HeapObject**> Pointers() override { return {HEAP_PTR(kValueOffset)}; }

    std::string ToString() override { return "Binding(" + (READ_VALUE(this, kValueOffset, JSValue*))->ToString() + ")"; }

   private:
    static constexpr size_t kValueOffset = kHeapObjectOffset;
    static constexpr size_t kCanDeleteOffset = kValueOffset + kPtrSize;
    static constexpr size_t kIsMutableOffset = kCanDeleteOffset + kBoolSize;
    static constexpr size_t kBindingOffset = kIsMutableOffset + kBoolSize;
  };

  static Handle<DeclarativeEnvironmentRecord> New() {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New(ENV_DECL, kPtrSize);
    auto bindings = HashMap<Binding>::New();

    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, bindings, HashMap<Binding>);

    new (env_rec.val()) DeclarativeEnvironmentRecord();
    return Handle<DeclarativeEnvironmentRecord>(env_rec);
  }

  std::vector<HeapObject**> Pointers() override { return {HEAP_PTR(kBindingsOffset)}; }

  HashMap<Binding>* bindings() { return READ_VALUE(this, kBindingsOffset, HashMap<Binding>*); }

  std::string ToString() override {
    return "DeclarativeEnvRec(" + log::ToString(this) + ")";
  }

 private:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
};

class ObjectEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<ObjectEnvironmentRecord> New(Handle<JSObject> obj, bool provide_this = false) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New(ENV_OBJ ,kPtrSize + kBoolSize);

    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, obj, JSObject);
    SET_VALUE(env_rec.val(), kProvideThisOffset, provide_this, bool);

    new (env_rec.val()) ObjectEnvironmentRecord();
    return Handle<ObjectEnvironmentRecord>(env_rec);
  }

  std::vector<HeapObject**> Pointers() override { return {HEAP_PTR(kBindingsOffset)}; }

  Handle<JSObject> bindings() { return READ_HANDLE_VALUE(this, kBindingsOffset, JSObject); }
  bool provide_this() { return READ_VALUE(this, kProvideThisOffset, bool); }

  virtual std::string ToString() override { return "ObjectEnvRec(" + log::ToString(this) + ")"; }

 private:
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