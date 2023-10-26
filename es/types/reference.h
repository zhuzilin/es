#ifndef ES_REFERENCE_H
#define ES_REFERENCE_H

#include <es/types/base.h>
#include <es/types/environment_record.h>
#include <es/types/builtin/global_object.h>
#include <es/types/error.h>

namespace es {

JSValue ToObject(JSValue& e, JSValue input);

namespace reference {

constexpr size_t kBaseOffset = 0;
constexpr size_t kReferenceNameOffset = kBaseOffset + sizeof(JSValue);

inline JSValue New(
  JSValue base,
  JSValue reference_name,
  bool strict_reference
) {
  ASSERT(reference_name.IsString());
  TEST_LOG("enter ref(" + JSValue::ToString(base) + "." + JSValue::ToString(reference_name) + ")");
  JSValue jsval;
  jsval.handle() = HeapObject::New(2 * sizeof(JSValue));

  bool has_primitive_base = base.IsBool() || base.IsString() || base.IsNumber();
  bool is_property_reference = base.IsObject() || has_primitive_base;
  bool is_unresolvable_reference = base.IsUndefined();

  jsval.header_.placeholder_.reference_header_.strict_reference_ = strict_reference;
  jsval.header_.placeholder_.reference_header_.has_primitive_base_ = has_primitive_base;
  jsval.header_.placeholder_.reference_header_.is_property_reference_ = is_property_reference;
  jsval.header_.placeholder_.reference_header_.is_unresolvable_reference_ = is_unresolvable_reference;

  SET_JSVALUE(jsval.handle().val(), kBaseOffset, base);
  SET_JSVALUE(jsval.handle().val(), kReferenceNameOffset, reference_name);

  jsval.SetType(JS_REF);
  return jsval;
}

inline JSValue GetBase(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kBaseOffset); }
inline JSValue GetReferencedName(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kReferenceNameOffset); }
inline bool IsStrictReference(JSValue jsval) {
  return jsval.header_.placeholder_.reference_header_.strict_reference_;
}
inline bool HasPrimitiveBase(JSValue jsval) {
  return jsval.header_.placeholder_.reference_header_.has_primitive_base_;
}
inline bool IsPropertyReference(JSValue jsval) {
  return jsval.header_.placeholder_.reference_header_.is_property_reference_;
}
inline bool IsUnresolvableReference(JSValue jsval) {
  return jsval.header_.placeholder_.reference_header_.is_unresolvable_reference_;
}

}  // namespace reference

JSValue GetValue(JSValue& e, JSValue V);
void PutValue(JSValue& e, JSValue V, JSValue W);

}  // namespace es

#endif