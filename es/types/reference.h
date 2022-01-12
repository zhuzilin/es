#ifndef ES_REFERENCE_H
#define ES_REFERENCE_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/builtin/global_object.h>
#include <es/types/conversion.h>
#include <es/error.h>

namespace es {

class Reference : public JSValue {
 public:
  Reference(
    JSValue* base,
    std::u16string reference_name,
    bool strict_reference
  ) : JSValue(JS_REF), base_(base), reference_name_(reference_name),
      strict_reference_(strict_reference) {}

  JSValue* GetBase() { return base_; }
  std::u16string GetReferencedName() { return reference_name_; }
  bool IsStrictReference() { return strict_reference_; }
  bool HasPrimitiveBase() {
    return base_->IsBool() || base_->IsString() || base_->IsNumber();
  }
  bool IsPropertyReference() {
    return base_->IsObject() || HasPrimitiveBase();
  }
  bool IsUnresolvableReference() { return base_->IsUndefined(); }

  std::string ToString() override { return "Reference"; }

 private:
  JSValue* base_;
  std::u16string reference_name_;
  bool strict_reference_;
};

JSValue* GetValue(Error* e, JSValue* V) {
  log::PrintSource("GetValue");
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
    // 4.a & 4.b
    if (!ref->HasPrimitiveBase()) {
      assert(base->IsObject());
      JSObject* obj = static_cast<JSObject*>(base);
      return obj->Get(e, ref->GetReferencedName());
    } else {
      JSObject* O = ToObject(e, base);
      if (e != nullptr)
        return nullptr;
      JSValue* tmp = O->GetProperty(ref->GetReferencedName());
      if (tmp->IsUndefined())
        return Undefined::Instance();
      PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
      if (desc->IsDataDescriptor()) {
        return desc->Value();
      } else {
        // TODO(zhuzilin)
      }
    }
  } else {
    assert(base->IsEnvironmentRecord());
    EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
    return er->GetBindingValue(e, ref->GetReferencedName(), ref->IsStrictReference());
  }
}

void PutValue(Error* e, JSValue* V, JSValue* W) {
  log::PrintSource("PutValue");
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
    GlobalObject::Instance()->Put(e, ref->GetReferencedName(), W, false);
  } else if (ref->IsPropertyReference()) {
    // TODO(zhuzilin)
    if (!ref->HasPrimitiveBase()) {
      
    } else {

    }
  } else {
    JSValue* base = ref->GetBase();
    assert(base->IsEnvironmentRecord());
    EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
    er->SetMutableBinding(e, ref->GetReferencedName(), W, ref->IsStrictReference());
  }
}

}  // namespace es

#endif