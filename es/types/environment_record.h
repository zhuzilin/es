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
    return static_cast<Handle<EnvironmentRecord>>(JSValue::New(JS_ENV_REC, size));
  }

  virtual bool HasBinding(Handle<String> N) = 0;
  virtual void CreateMutableBinding(Error* e, Handle<String> N, bool D) = 0;
  virtual void SetMutableBinding(Error* e, Handle<String> N, Handle<JSValue> V, bool S) = 0;
  virtual Handle<JSValue> GetBindingValue(Error* e, Handle<String> N, bool S) = 0;
  virtual bool DeleteBinding(Error* e, Handle<String> N) = 0;
  virtual Handle<JSValue> ImplicitThisValue() = 0;

 protected:
  static constexpr size_t kEnvironmentRecordOffset = kJSValueOffset;
};

class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  class Binding : public HeapObject {
   public:
    static Handle<Binding> New(Handle<JSValue> value, bool can_delete, bool is_mutable) {
      std::cout << "Binding::New" << std::endl;
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
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New(kPtrSize);
    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, HashMap<Binding>::New(), HashMap<Binding>);
    new (env_rec.val()) DeclarativeEnvironmentRecord();
    return Handle<DeclarativeEnvironmentRecord>(env_rec);
  }

  std::vector<HeapObject**> Pointers() override { return {HEAP_PTR(kBindingsOffset)}; }

  HashMap<Binding>* bindings() { return READ_VALUE(this, kBindingsOffset, HashMap<Binding>*); }

  bool HasBinding(Handle<String> N) override {
    return bindings()->GetRaw(N) != nullptr;
  }

  void CreateMutableBinding(Error* e, Handle<String> N, bool D) override {
    assert(!HasBinding(N));
    Handle<Binding> b = Binding::New(Undefined::Instance(), D, true);
    bindings()->Set(N, b);
  }

  void SetMutableBinding(Error* e, Handle<String> N, Handle<JSValue> V, bool S) override {
    log::PrintSource("enter SetMutableBinding ", N.val()->data(), " to " + V.ToString());
    assert(V.val()->IsLanguageType());
    assert(HasBinding(N));
    // NOTE(zhuzilin) If we do note b = bindings_[N] and change b.value,
    // the value stored in bindings_ won't change.
    if (bindings()->GetRaw(N)->is_mutable()) {
      bindings()->GetRaw(N)->SetValue(V);
    } else if (S) {
      *e = *Error::TypeError();
    }
  }

  Handle<JSValue> GetBindingValue(Error* e, Handle<String> N, bool S) override {
    assert(HasBinding(N));
    Binding* b = bindings()->GetRaw(N);
    if (b->value().val()->IsUndefined() && !b->is_mutable()) {
      if (S) {
        *e = *Error::ReferenceError(N.val()->data() + u" is not defined");
        return Handle<JSValue>();
      } else {
        log::PrintSource("GetBindingValue ", N.val()->data(), " undefined");
        return Undefined::Instance();
      }
    }
    log::PrintSource("GetBindingValue ", N.val()->data(), " " + b->value().ToString());
    return b->value();
  }

  bool DeleteBinding(Error* e, Handle<String> N) override {
    if (!HasBinding(N)) return true;
    if (!bindings()->GetRaw(N)->can_delete()) {
      return false;
    }
    bindings()->Delete(N);
    return true;
  }

  Handle<JSValue> ImplicitThisValue() override {
    return Undefined::Instance();
  }

  void CreateImmutableBinding(Handle<String> N) {
    assert(!HasBinding(N));
    Handle<Binding> b = Binding::New(Undefined::Instance(), false, false);
    bindings()->Set(N, b);
  }

  void InitializeImmutableBinding(Handle<String> N, Handle<JSValue> V) {
    assert(HasBinding(N));
    assert(!bindings()->GetRaw(N)->is_mutable() &&
           bindings()->GetRaw(N)->value().val()->IsUndefined());
    bindings()->GetRaw(N)->SetValue(V);
  }

  std::string ToString() override {
    return "DeclarativeEnvRec(" + log::ToString(this) + ")";
  }

 private:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
};

class ObjectEnvironmentRecord : public EnvironmentRecord {
 public:
  static Handle<ObjectEnvironmentRecord> New(Handle<JSObject> obj, bool provide_this = false) {
    Handle<EnvironmentRecord> env_rec = EnvironmentRecord::New(kPtrSize + kBoolSize);
    SET_HANDLE_VALUE(env_rec.val(), kBindingsOffset, obj, JSObject);
    SET_VALUE(env_rec.val(), kProvideThisOffset, provide_this, bool);
    new (env_rec.val()) ObjectEnvironmentRecord();
    return Handle<ObjectEnvironmentRecord>(env_rec);
  }

  std::vector<HeapObject**> Pointers() override { return {HEAP_PTR(kBindingsOffset)}; }

  Handle<JSObject> bindings() { return READ_HANDLE_VALUE(this, kBindingsOffset, JSObject); }
  bool provide_this() { return READ_VALUE(this, kProvideThisOffset, bool); }

  bool HasBinding(Handle<String> N) override {
    return HasProperty(bindings(), N);
  }

  // 10.2.1.2.2 CreateMutableBinding (N, D)
  void CreateMutableBinding(Error* e, Handle<String> N, bool D) override {
    assert(!HasBinding(N));
    Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
    desc.val()->SetDataDescriptor(Undefined::Instance(), true, true, D);
    DefineOwnProperty(e, bindings(), N, desc, true);
  }

  void SetMutableBinding(Error* e, Handle<String> N, Handle<JSValue> V, bool S) override {
    log::PrintSource("enter SetMutableBinding " + N.ToString() + " to " + V.ToString());
    assert(V.val()->IsLanguageType());
    Put(e, bindings(), N, V, S);
  }

  Handle<JSValue> GetBindingValue(Error* e, Handle<String> N, bool S) override {
    bool value = HasBinding(N);
    if (!value) {
      if (S) {
        *e = *Error::ReferenceError(N.val()->data() + u" is not defined");
        return Handle<JSValue>();
      } else {
        return Undefined::Instance();
      }
    }
    return Get(e, bindings(), N);
  }

  bool DeleteBinding(Error* e, Handle<String> N) override {
    return Delete(e, bindings(), N, false);
  }

  Handle<JSValue> ImplicitThisValue() override {
    if (provide_this()) {
      return bindings();
    }
    return Undefined::Instance();
  }

  virtual std::string ToString() override { return "ObjectEnvRec(" + log::ToString(this) + ")"; }

 private:
  static constexpr size_t kBindingsOffset = kEnvironmentRecordOffset;
  static constexpr size_t kProvideThisOffset = kBindingsOffset + kPtrSize;
};

}  // namespace es

#endif  // ES_ENVIRONMENT_RECORD_H