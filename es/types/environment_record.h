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
  EnvironmentRecord() : JSValue(JS_ENV_REC) {}

  virtual bool HasBinding(std::u16string_view N) = 0;
  virtual void CreateMutableBinding(std::u16string_view N, bool D, Error* e) = 0;
  virtual void SetMutableBinding(std::u16string_view N, JSValue* V, bool S, Error* e) = 0;
  virtual JSValue* GetBindingValue(std::u16string_view N, bool S, Error* e) = 0;
  virtual bool DeleteBinding(std::u16string_view N, Error* e) = 0;
  virtual JSValue* ImplicitThisValue() = 0;
};

class DeclarativeEnvironmentRecord : public EnvironmentRecord {
 public:
  struct Binding {
    JSValue* value;
    bool can_delete;
    bool is_mutable;
  };

  bool HasBinding(std::u16string_view N) override {
    return bindings_.find(N) != bindings_.end();
  }

  void CreateMutableBinding(std::u16string_view N, bool D, Error* e) override {
    assert(!HasBinding(N));
    Binding b;
    b.value = Undefined::Instance();
    b.can_delete = D;
    b.is_mutable = true;
    bindings_[N] = b;
  }

  void SetMutableBinding(std::u16string_view N, JSValue* V, bool S, Error* e) override {
    assert(V->IsLanguageType());
    assert(HasBinding(N));
    Binding b = bindings_[N];
    if (b.is_mutable) {
      b.value = V;
    } else if (S) {
      e = Error::TypeError();
    }
  }

  JSValue* GetBindingValue(std::u16string_view N, bool S, Error* e) override {
    assert(HasBinding(N));
    Binding b = bindings_[N];
    if (b.value->IsUndefined()) {
      if (S) {
        e = Error::ReferenceError();
        return nullptr;
      } else {
        return Undefined::Instance();
      }
    }
    return b.value;
  }

  bool DeleteBinding(std::u16string_view N, Error* e) override {
    if (!HasBinding(N)) return true;
    Binding b = bindings_[N];
    if (!b.can_delete) {
      return false;
    }
    bindings_.erase(N);
    return true;
  }

  JSValue* ImplicitThisValue() override {
    return Undefined::Instance();
  }

  void CreateImmutableBinding(std::u16string_view N) {
    assert(!HasBinding(N));
    Binding b;
    b.value = Undefined::Instance();
    b.can_delete = false;
    b.is_mutable = false;
    bindings_[N] = b;
  }

  void InitializeImmutableBinding(std::u16string_view N, JSValue* V) {
    assert(!HasBinding(N));
    Binding b = bindings_[N];
    assert(!b.is_mutable && b.value->IsUndefined());
    b.value = V;
  }

 private:
  std::unordered_map<std::u16string_view, Binding> bindings_;
};

class ObjectEnvironmentRecord : public EnvironmentRecord {
 public:
  ObjectEnvironmentRecord(JSObject* obj) : bindings_(obj), provide_this_(false) {}

  bool HasBinding(std::u16string_view N) override {
    return bindings_->HasProperty(N);
  }

  void CreateMutableBinding(std::u16string_view N, bool D, Error* e) override {
    assert(!HasBinding(N));
    PropertyDescriptor* desc = new PropertyDescriptor();
    desc->SetDataDescriptor(Undefined::Instance(), true, true, D);
    bindings_->DefineOwnProperty(N, desc, true, e);
  }

  void SetMutableBinding(std::u16string_view N, JSValue* V, bool S, Error* e) override {
    assert(V->IsLanguageType());
    bindings_->Put(N, V, S, e);
  }

  JSValue* GetBindingValue(std::u16string_view N, bool S, Error* e) override {
    bool value = HasBinding(N);
    if (!value) {
      if (S) {
        e = Error::ReferenceError();
        return nullptr;
      } else {
        return Undefined::Instance();
      }
    }
    return bindings_->Get(N);
  }

  bool DeleteBinding(std::u16string_view N, Error* e) override {
    return bindings_->Delete(N, false, e);
  }

  JSValue* ImplicitThisValue() override {
    if (provide_this_) {
      return bindings_;
    }
    return Undefined::Instance();
  }

 private:
  JSObject *bindings_;
  bool provide_this_;
};

}  // namespace es

#endif  // ES_ENVIRONMENT_RECORD_H