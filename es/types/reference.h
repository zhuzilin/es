#ifndef ES_REFERENCE_H
#define ES_REFERENCE_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/builtin/global_object.h>
#include <es/error.h>

namespace es {

JSObject* ToObject(Error* e, JSValue* input);

class Reference : public JSValue {
 public:
  static Reference* New(
    JSValue* base,
    String* reference_name,
    bool strict_reference
  ) {
    JSValue* jsval = JSValue::New(JS_REF, kStrictReferenceOffset + kBoolSize - kJSValueOffset);
    SET_VALUE(jsval, kBaseOffset, base, JSValue*);
    SET_VALUE(jsval, kReferenceNameOffset, reference_name, String*);
    SET_VALUE(jsval, kStrictReferenceOffset, strict_reference, bool);
    return new (jsval) Reference();
  }

  JSValue* GetBase() { return READ_VALUE(this, kBaseOffset, JSValue*); }
  String* GetReferencedName() { return READ_VALUE(this, kReferenceNameOffset, String*); }
  bool IsStrictReference() { return READ_VALUE(this, kStrictReferenceOffset, bool); }
  bool HasPrimitiveBase() {
    return GetBase()->IsBool() || GetBase()->IsString() || GetBase()->IsNumber();
  }
  bool IsPropertyReference() {
    return GetBase()->IsObject() || HasPrimitiveBase();
  }
  bool IsUnresolvableReference() { return GetBase()->IsUndefined(); }

  std::string ToString() override { return "ref(" + GetReferencedName()->ToString() + ")"; }

  std::vector<void*> Pointers() override {
    assert(false);
  }

 private:
  static constexpr size_t kBaseOffset = kJSValueOffset;
  static constexpr size_t kReferenceNameOffset = kBaseOffset + kPtrSize;
  static constexpr size_t kStrictReferenceOffset = kReferenceNameOffset + kPtrSize;
};

JSValue* GetValue(Error* e, JSValue* V) {
  log::PrintSource("GetValue V:" + V->ToString());
  if (!V->IsReference()) {
    return V;
  }
  Reference* ref = static_cast<Reference*>(V);
  if (ref->IsUnresolvableReference()) {
    *e = *Error::ReferenceError(ref->GetReferencedName()->data() + u" is not defined");
    return nullptr;
  }
  JSValue* base = ref->GetBase();
  if (ref->IsPropertyReference()) {  // 4
    // 4.a & 4.b
    if (!ref->HasPrimitiveBase()) {
      assert(base->IsObject());
      JSObject* obj = static_cast<JSObject*>(base);
      return obj->Get(e, ref->GetReferencedName());
    } else {  // special [[Get]]
      JSObject* O = ToObject(e, base);
      if (!e->IsOk()) return nullptr;
      JSValue* tmp = O->GetProperty(ref->GetReferencedName());
      if (tmp->IsUndefined())
        return Undefined::Instance();
      PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
      if (desc->IsDataDescriptor()) {
        return desc->Value();
      } else {
        assert(desc->IsAccessorDescriptor());
        JSValue* getter = desc->Get();
        if (getter->IsUndefined()) {
          return Undefined::Instance();
        }
        JSObject* getter_obj = static_cast<JSObject*>(getter);
        return getter_obj->Call(e, base, {});
      }
    }
  } else {
    assert(base->IsEnvironmentRecord());
    EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
    return er->GetBindingValue(e, ref->GetReferencedName(), ref->IsStrictReference());
  }
}

void PutValue(Error* e, JSValue* V, JSValue* W) {
  log::PrintSource("PutValue V: " + V->ToString() + ", W: " + W->ToString());
  if (!V->IsReference()) {
    *e = *Error::ReferenceError();
    return;
  }
  Reference* ref = static_cast<Reference*>(V);
  JSValue* base = ref->GetBase();
  if (ref->IsUnresolvableReference()) {  // 3
    if (ref->IsStrictReference()) {  // 3.a
      *e = *Error::ReferenceError();
      return;
    }
    GlobalObject::Instance()->Put(e, ref->GetReferencedName(), W, false);  // 3.b
  } else if (ref->IsPropertyReference()) {
    bool throw_flag = ref->IsStrictReference();
    String* P = ref->GetReferencedName();
    if (!ref->HasPrimitiveBase()) {
      assert(base->IsObject());
      JSObject* base_obj = static_cast<JSObject*>(base);
      base_obj->Put(e, P, W, throw_flag);
    } else {  // special [[Put]]
      JSObject* O = ToObject(e, base);
      if (!O->CanPut(P)) {  // 2
        if (throw_flag)
          *e = *Error::TypeError();
        return;
      }
      JSValue* tmp = O->GetOwnProperty(P);  // 3
      if(!tmp->IsUndefined()) {
        PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(tmp);
        if (own_desc->IsDataDescriptor()) {  // 4
          if (throw_flag)
            *e = *Error::TypeError();
          return;
        }
      }
      tmp = O->GetProperty(P);
      if (!tmp->IsUndefined()) {
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
        if (desc->IsAccessorDescriptor()) {  // 4
          JSValue* setter = desc->Set();
          assert(!setter->IsUndefined());
          JSObject* setter_obj = static_cast<JSObject*>(setter);
          setter_obj->Call(e, base, {W});
        } else {  // 7
          if (throw_flag)
            *e = *Error::TypeError();
          return;
        }
      }
    }
  } else {
    assert(base->IsEnvironmentRecord());
    EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
    er->SetMutableBinding(e, ref->GetReferencedName(), W, ref->IsStrictReference());
  }
}

}  // namespace es

#endif