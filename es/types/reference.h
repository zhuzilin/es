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
    Handle<JSValue> jsval = HeapObject::New(kStrictReferenceOffset + kBoolSize - kJSValueOffset);

    SET_HANDLE_VALUE(jsval.val(), kBaseOffset, base, JSValue);
    SET_HANDLE_VALUE(jsval.val(), kReferenceNameOffset, reference_name, String);
    SET_VALUE(jsval.val(), kStrictReferenceOffset, strict_reference, bool);

    jsval.val()->SetType(JS_REF);
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

 public:
  static constexpr size_t kBaseOffset = kJSValueOffset;
  static constexpr size_t kReferenceNameOffset = kBaseOffset + kPtrSize;
  static constexpr size_t kStrictReferenceOffset = kReferenceNameOffset + kPtrSize;
};

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V);
void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W);

}  // namespace es

#endif