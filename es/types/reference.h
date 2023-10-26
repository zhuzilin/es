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
    Handle<JSValue> jsval = HeapObject::New(kBitmaskOffset + kCharSize - kJSValueOffset);

    Type base_type = base.val()->type();
    char bitmask = 0;
    if (strict_reference)
      bitmask |= STRICT;
    bool has_primitive_base = IsBool(base_type) ||
                              IsString(base_type) ||
                              IsNumber(base_type);
    if (has_primitive_base)
      bitmask |= HAS_PRIMITIVE_BASE;
    bool is_property_reference = has_primitive_base || IsObject(base_type);
    if (is_property_reference)
      bitmask |= PROPERTY;
    bool is_unresolvable_reference = IsUndefined(base_type);
    if (is_unresolvable_reference)
      bitmask |= UNRESOLVABLE;

    SET_HANDLE_VALUE(jsval.val(), kBaseOffset, base, JSValue);
    SET_HANDLE_VALUE(jsval.val(), kReferenceNameOffset, reference_name, String);
    SET_VALUE(jsval.val(), kBitmaskOffset, bitmask, char);

    jsval.val()->SetType(JS_REF);
    return Handle<Reference>(jsval);
  }

  Handle<JSValue> GetBase() { return READ_HANDLE_VALUE(this, kBaseOffset, JSValue); }
  Handle<String> GetReferencedName() { return READ_HANDLE_VALUE(this, kReferenceNameOffset, String); }
  inline char bitmask() {
    return READ_VALUE(this, kBitmaskOffset, char);
  }
  bool IsStrictReference() { return bitmask() & STRICT; }
  bool HasPrimitiveBase() { return bitmask() & HAS_PRIMITIVE_BASE; }
  bool IsPropertyReference() { return bitmask() & PROPERTY; }
  bool IsUnresolvableReference() { return bitmask() & UNRESOLVABLE; }

 public:
  static constexpr size_t kBaseOffset = kJSValueOffset;
  static constexpr size_t kReferenceNameOffset = kBaseOffset + kPtrSize;
  static constexpr size_t kBitmaskOffset = kReferenceNameOffset + kPtrSize;

  enum Field {
    STRICT              = 1 << 0,
    HAS_PRIMITIVE_BASE  = 1 << 1,
    PROPERTY            = 1 << 2,
    UNRESOLVABLE        = 1 << 3,
  };
};

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V);
void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W);

}  // namespace es

#endif