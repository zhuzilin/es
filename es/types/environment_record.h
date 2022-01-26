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
  static EnvironmentRecord* New(size_t size) {
    return static_cast<EnvironmentRecord*>(JSValue::New(JS_ENV_REC, size));
  }

  virtual bool HasBinding(String* N) = 0;
  virtual void CreateMutableBinding(Error* e, String* N, bool D) = 0;
  virtual void SetMutableBinding(Error* e, String* N, JSValue* V, bool S) = 0;
  virtual JSValue* GetBindingValue(Error* e, String* N, bool S) = 0;
  virtual bool DeleteBinding(Error* e, String* N) = 0;
  virtual JSValue* ImplicitThisValue() = 0;

 protected:
  static constexpr size_t kEnvironmentRecordOffset = kJSValueOffset;
};

class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  class Binding : public HeapObject {
   public:
    static Binding* New(JSValue* value, bool can_delete, bool is_mutable) {
      HeapObject* heap_obj = HeapObject::New(kBindingOffset - kHeapObjectOffset);
      SET_VALUE(heap_obj, kValueOffset, value, JSValue*);
      SET_VALUE(heap_obj, kCanDeleteOffset, can_delete, bool);
      SET_VALUE(heap_obj, kIsMutableOffset, is_mutable, bool);
      return new (heap_obj) Binding();
    }

    JSValue* value() { return READ_VALUE(this, kValueOffset, JSValue*); };
    void SetValue(JSValue* value) { SET_VALUE(this, kValueOffset, value, JSValue*); }
    bool can_delete() { return READ_VALUE(this, kCanDeleteOffset, bool); }
    bool is_mutable() { return READ_VALUE(this, kIsMutableOffset, bool); }

    std::vector<void*> Pointers() override { return {}; }

   private:
    static constexpr size_t kValueOffset = kHeapObjectOffset;
    static constexpr size_t kCanDeleteOffset = kValueOffset + kPtrSize;
    static constexpr size_t kIsMutableOffset = kCanDeleteOffset + kBoolSize;
    static constexpr size_t kBindingOffset = kIsMutableOffset + kBoolSize;
  };

  static DeclarativeEnvironmentRecord* New() {
    EnvironmentRecord* env_rec = EnvironmentRecord::New(kPtrSize);
    SET_VALUE(env_rec, kBindingsOffset, HashMap<Binding>::New(), HashMap<Binding>*);
    return new (env_rec) DeclarativeEnvironmentRecord();
  }

  HashMap<Binding>* bindings() { return READ_VALUE(this, kBindingsOffset, HashMap<Binding>*); }

  bool HasBinding(String* N) override {
    return bindings()->Get(N) != nullptr;
  }

  void CreateMutableBinding(Error* e, String* N, bool D) override {
    assert(!HasBinding(N));
    Binding* b = Binding::New(Undefined::Instance(), D, true);
    bindings()->Set(N, b);
  }

  void SetMutableBinding(Error* e, String* N, JSValue* V, bool S) override {
    log::PrintSource("enter SetMutableBinding ", N->data(), " to " + V->ToString());
    assert(V->IsLanguageType());
    assert(HasBinding(N));
    // NOTE(zhuzilin) If we do note b = bindings_[N] and change b.value,
    // the value stored in bindings_ won't change.
    if (bindings()->Get(N)->is_mutable()) {
      bindings()->Get(N)->SetValue(V);
    } else if (S) {
      *e = *Error::TypeError();
    }
  }

  JSValue* GetBindingValue(Error* e, String* N, bool S) override {
    assert(HasBinding(N));
    Binding* b = bindings()->Get(N);
    if (b->value()->IsUndefined() && !b->is_mutable()) {
      if (S) {
        *e = *Error::ReferenceError(N->data() + u" is not defined");
        return nullptr;
      } else {
        log::PrintSource("GetBindingValue ", N->data(), " undefined");
        return Undefined::Instance();
      }
    }
    log::PrintSource("GetBindingValue ", N->data(), " " + b->value()->ToString());
    return b->value();
  }

  bool DeleteBinding(Error* e, String* N) override {
    if (!HasBinding(N)) return true;
    if (!bindings()->Get(N)->can_delete()) {
      return false;
    }
    bindings()->Delete(N);
    return true;
  }

  JSValue* ImplicitThisValue() override {
    return Undefined::Instance();
  }

  void CreateImmutableBinding(String* N) {
    assert(!HasBinding(N));
    Binding* b = Binding::New(Undefined::Instance(), false, false);
    bindings()->Set(N, b);
  }

  void InitializeImmutableBinding(String* N, JSValue* V) {
    assert(HasBinding(N));
    assert(!bindings()->Get(N)->is_mutable() && bindings()->Get(N)->value()->IsUndefined());
    bindings()->Get(N)->SetValue(V);
  }

  std::string ToString() override {
    return "DeclarativeEnvRec(" + log::ToString(this) + ")";
  }

  std::vector<void*> Pointers() override {
    assert(false);
  }

 private:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
};

class ObjectEnvironmentRecord : public EnvironmentRecord {
 public:
  static ObjectEnvironmentRecord* New(JSObject* obj, bool provide_this = false) {
    EnvironmentRecord* env_rec = EnvironmentRecord::New(kPtrSize + kBoolSize);
    SET_VALUE(env_rec, kBindingsOffset, obj, JSObject*);
    SET_VALUE(env_rec, kProvideThisOffset, provide_this, bool);
    return new (env_rec) ObjectEnvironmentRecord();
  }

  JSObject* bindings() { return READ_VALUE(this, kBindingsOffset, JSObject*); }
  bool provide_this() { return READ_VALUE(this, kProvideThisOffset, bool); }

  bool HasBinding(String* N) override {
    return bindings()->HasProperty(N);
  }

  // 10.2.1.2.2 CreateMutableBinding (N, D)
  void CreateMutableBinding(Error* e, String* N, bool D) override {
    assert(!HasBinding(N));
    PropertyDescriptor* desc = PropertyDescriptor::New();
    desc->SetDataDescriptor(Undefined::Instance(), true, true, D);
    bindings()->DefineOwnProperty(e, N, desc, true);
  }

  void SetMutableBinding(Error* e, String* N, JSValue* V, bool S) override {
    log::PrintSource("enter SetMutableBinding " + N->ToString() + " to " + V->ToString());
    assert(V->IsLanguageType());
    bindings()->Put(e, N, V, S);
  }

  JSValue* GetBindingValue(Error* e, String* N, bool S) override {
    bool value = HasBinding(N);
    if (!value) {
      if (S) {
        *e = *Error::ReferenceError(N->data() + u" is not defined");
        return nullptr;
      } else {
        return Undefined::Instance();
      }
    }
    return bindings()->Get(e, N);
  }

  bool DeleteBinding(Error* e, String* N) override {
    return bindings()->Delete(e, N, false);
  }

  JSValue* ImplicitThisValue() override {
    if (provide_this()) {
      return bindings();
    }
    return Undefined::Instance();
  }

  virtual std::string ToString() override { return "ObjectEnvRec(" + log::ToString(this) + ")"; }

  std::vector<void*> Pointers() override {
    assert(false);
  }

 private:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
  static constexpr size_t kProvideThisOffset = kBindingsOffset + kPtrSize;
};

}  // namespace es

#endif  // ES_ENVIRONMENT_RECORD_H