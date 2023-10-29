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
    Handle<JSValue> jsval = HeapObject::New(kStrictOffset + kBoolSize - kJSValueOffset);

    SET_HANDLE_VALUE(jsval.val(), kBaseOffset, base, JSValue);
    SET_HANDLE_VALUE(jsval.val(), kReferenceNameOffset, reference_name, String);
    SET_VALUE(jsval.val(), kStrictOffset, strict_reference, bool);

    jsval.val()->SetType(JS_REF);
    return Handle<Reference>(jsval);
  }

  Handle<JSValue> GetBase() { return READ_HANDLE_VALUE(this, kBaseOffset, JSValue); }
  Handle<String> GetReferencedName() { return READ_HANDLE_VALUE(this, kReferenceNameOffset, String); }
  bool IsStrictReference() { return READ_VALUE(this, kStrictOffset, bool); }

  static bool HasPrimitiveBase(Handle<JSValue> base) {
    Type base_type = base.val()->type();
    return IsBool(base_type) || IsString(base_type) || IsNumber(base_type);
  }
  static bool IsPropertyReference(Handle<JSValue> base) { return base.val()->IsObject() || HasPrimitiveBase(base); }
  static bool IsUnresolvableReference(Handle<JSValue> base) { return base.val()->IsUndefined(); }

 public:
  static constexpr size_t kBaseOffset = kJSValueOffset;
  static constexpr size_t kReferenceNameOffset = kBaseOffset + kPtrSize;
  static constexpr size_t kStrictOffset = kReferenceNameOffset + kPtrSize;

  enum Field {
    STRICT              = 1 << 0,
    HAS_PRIMITIVE_BASE  = 1 << 1,
    PROPERTY            = 1 << 2,
    UNRESOLVABLE        = 1 << 3,
  };
};

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V);
void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W);
Handle<JSValue> GetValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict);
void PutValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict, Handle<JSValue> value);

}  // namespace es

#endif