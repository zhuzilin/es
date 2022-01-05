#ifndef ES_REFERENCE_H
#define ES_REFERENCE_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/error.h>

namespace es {

class Reference : public JSValue {
 public:
  Reference(
    JSValue* base,
    std::u16string_view reference_name,
    bool strict_reference
  ) : JSValue(JS_REF), base_(base), reference_name_(reference_name),
      strict_reference_(strict_reference) {}

  JSValue* GetBase() { return base_; }
  std::u16string_view GetReferencedName() { return reference_name_; }
  bool IsStrictReference() { return strict_reference_; }
  bool HashPrimitiveBase() {
    return base_->IsBool() || base_->IsString() || base_->IsNumber();
  }
  bool IsPropertyReference() {
    return base_->IsObject() || HashPrimitiveBase();
  }
  bool IsUnresolvableReference() { return base_->IsUndefined(); }

 private:
  JSValue* base_;
  std::u16string_view reference_name_;
  bool strict_reference_;
};

JSValue* GetValue(JSValue* V, Error* e) {
  if (!V->IsReference()) {
    return V;
  }
  Reference* ref = static_cast<Reference*>(V);
  if (ref->IsUnresolvableReference()) {
    e = Error::ReferenceError();
    return nullptr;
  }
  JSValue* base = ref->GetBase();
  if (ref->IsPropertyReference()) {
    // TODO(zhuzilin)
    if (!ref->HashPrimitiveBase()) {
      
    } else {

    }
  } else {
    assert(base->IsEnvironmentRecord());
    EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
    return er->GetBindingValue(ref->GetReferencedName(), ref->IsStrictReference());
  }
}

void PutValue(JSValue* V, JSValue* W, Error* e) {
  if (!V->IsReference()) {
    e = Error::ReferenceError();
    return;
  }
  Reference* ref = static_cast<Reference*>(V);
  if (ref->IsUnresolvableReference()) {
    if (ref->IsStrictReference()) {
      e = Error::ReferenceError();
      return;
    }
    // TODO(zhuzilin)
  }
  JSValue* base = ref->GetBase();
  if (ref->IsPropertyReference()) {
    // TODO(zhuzilin)
    if (!ref->HashPrimitiveBase()) {
      
    } else {

    }
  } else {
    assert(base->IsEnvironmentRecord());
    EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
    er->SetMutableBinding(ref->GetReferencedName(), W, ref->IsStrictReference());
  }
}

}  // namespace es

#endif