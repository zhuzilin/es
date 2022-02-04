#ifndef ES_REFERENCE_H
#define ES_REFERENCE_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/builtin/global_object.h>
#include <es/error.h>

namespace es {

Handle<JSObject> ToObject(Handle<Error>& e, Handle<JSValue> input);

class Reference : public JSValue {
 public:
  static Handle<Reference> New(
    Handle<JSValue> base,
    Handle<String> reference_name,
    bool strict_reference
  ) {
    Handle<JSValue> jsval = JSValue::New(JS_REF, kStrictReferenceOffset + kBoolSize - kJSValueOffset);

    SET_HANDLE_VALUE(jsval.val(), kBaseOffset, base, JSValue);
    SET_HANDLE_VALUE(jsval.val(), kReferenceNameOffset, reference_name, String);
    SET_VALUE(jsval.val(), kStrictReferenceOffset, strict_reference, bool);

    new (jsval.val()) Reference();
    return Handle<Reference>(jsval);
  }

  Handle<JSValue> GetBase() { return READ_HANDLE_VALUE(this, kBaseOffset, JSValue); }
  Handle<String> GetReferencedName() { return READ_HANDLE_VALUE(this, kReferenceNameOffset, String); }
  bool IsStrictReference() { return READ_VALUE(this, kStrictReferenceOffset, bool); }
  bool HasPrimitiveBase() {
    return GetBase().val()->IsBool() || GetBase().val()->IsString() || GetBase().val()->IsNumber();
  }
  bool IsPropertyReference() {
    return GetBase().val()->IsObject() || HasPrimitiveBase();
  }
  bool IsUnresolvableReference() { return GetBase().val()->IsUndefined(); }

  std::string ToString() override {
    String* name = READ_VALUE(this, kReferenceNameOffset, String*);
    return "ref(" + name->ToString() + ")";
  }

  std::vector<HeapObject**> Pointers() override {
    return {HEAP_PTR(kBaseOffset), HEAP_PTR(kReferenceNameOffset)};
  }

 private:
  static constexpr size_t kBaseOffset = kJSValueOffset;
  static constexpr size_t kReferenceNameOffset = kBaseOffset + kPtrSize;
  static constexpr size_t kStrictReferenceOffset = kReferenceNameOffset + kPtrSize;
};

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V) {
  log::PrintSource("GetValue V:" + V.ToString());
  if (!V.val()->IsReference()) {
    return V;
  }
  Handle<Reference> ref = static_cast<Handle<Reference>>(V);
  if (ref.val()->IsUnresolvableReference()) {
    e = Error::ReferenceError(ref.val()->GetReferencedName().val()->data() + u" is not defined");
    return Handle<JSValue>();
  }
  Handle<JSValue> base = ref.val()->GetBase();
  if (ref.val()->IsPropertyReference()) {  // 4
    // 4.a & 4.b
    if (!ref.val()->HasPrimitiveBase()) {
      assert(base.val()->IsObject());
      Handle<JSObject> obj = static_cast<Handle<JSObject>>(base);
      return Get(e, obj, ref.val()->GetReferencedName());
    } else {  // special [[Get]]
      Handle<JSObject> O = ToObject(e, base);
      if (!e.val()->IsOk()) return Handle<JSValue>();
      Handle<JSValue> tmp = GetProperty(O, ref.val()->GetReferencedName());
      if (tmp.val()->IsUndefined())
        return Undefined::Instance();
      Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(tmp);
      if (desc.val()->IsDataDescriptor()) {
        return desc.val()->Value();
      } else {
        assert(desc.val()->IsAccessorDescriptor());
        Handle<JSValue> getter = desc.val()->Get();
        if (getter.val()->IsUndefined()) {
          return Undefined::Instance();
        }
        Handle<JSObject> getter_obj = static_cast<Handle<JSObject>>(getter);
        return Call(e, getter_obj, base, {});
      }
    }
  } else {
    assert(base.val()->IsEnvironmentRecord());
    Handle<EnvironmentRecord> er = static_cast<Handle<EnvironmentRecord>>(base);
    return GetBindingValue(e, er, ref.val()->GetReferencedName(), ref.val()->IsStrictReference());
  }
}

void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W) {
  log::PrintSource("PutValue V: " + V.ToString() + ", W: " + W.ToString());
  if (!V.val()->IsReference()) {
    e = Error::ReferenceError(u"put value to non-reference.");
    return;
  }
  Handle<Reference> ref = static_cast<Handle<Reference>>(V);
  Handle<JSValue> base = ref.val()->GetBase();
  if (ref.val()->IsUnresolvableReference()) {  // 3
    if (ref.val()->IsStrictReference()) {  // 3.a
      e = Error::ReferenceError(ref.val()->GetReferencedName().val()->data() + u" is not defined");
      return;
    }
    Put(e, GlobalObject::Instance(), ref.val()->GetReferencedName(), W, false);  // 3.b
  } else if (ref.val()->IsPropertyReference()) {
    bool throw_flag = ref.val()->IsStrictReference();
    Handle<String> P = ref.val()->GetReferencedName();
    if (!ref.val()->HasPrimitiveBase()) {
      assert(base.val()->IsObject());
      Handle<JSObject> base_obj = static_cast<Handle<JSObject>>(base);
      Put(e, base_obj, P, W, throw_flag);
    } else {  // special [[Put]]
      Handle<JSObject> O = ToObject(e, base);
      if (!CanPut(O, P)) {  // 2
        if (throw_flag)
          e = Error::TypeError();
        return;
      }
      Handle<JSValue> tmp = GetOwnProperty(O, P);  // 3
      if(!tmp.val()->IsUndefined()) {
        Handle<PropertyDescriptor> own_desc = static_cast<Handle<PropertyDescriptor>>(tmp);
        if (own_desc.val()->IsDataDescriptor()) {  // 4
          if (throw_flag)
            e = Error::TypeError();
          return;
        }
      }
      tmp = GetProperty(O, P);
      if (!tmp.val()->IsUndefined()) {
        Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(tmp);
        if (desc.val()->IsAccessorDescriptor()) {  // 4
          Handle<JSValue> setter = desc.val()->Set();
          assert(!setter.val()->IsUndefined());
          Handle<JSObject> setter_obj = static_cast<Handle<JSObject>>(setter);
          Call(e, setter_obj, base, {W});
        } else {  // 7
          if (throw_flag)
            e = Error::TypeError();
          return;
        }
      }
    }
  } else {
    assert(base.val()->IsEnvironmentRecord());
    Handle<EnvironmentRecord> er = static_cast<Handle<EnvironmentRecord>>(base);
    SetMutableBinding(e, er, ref.val()->GetReferencedName(), W, ref.val()->IsStrictReference());
  }
}

}  // namespace es

#endif