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
constexpr size_t kStrictReferenceOffset = kReferenceNameOffset + sizeof(JSValue);

inline JSValue New(
  JSValue base,
  JSValue reference_name,
  bool strict_reference
) {
  ASSERT(reference_name.IsString());
  TEST_LOG("enter ref(" + JSValue::ToString(base) + "." + JSValue::ToString(reference_name) + ")");
  JSValue jsval;
  jsval.handle() = HeapObject::New(kStrictReferenceOffset + kBoolSize);

  SET_JSVALUE(jsval.handle().val(), kBaseOffset, base);
  SET_JSVALUE(jsval.handle().val(), kReferenceNameOffset, reference_name);
  SET_VALUE(jsval.handle().val(), kStrictReferenceOffset, strict_reference, bool);

  jsval.SetType(JS_REF);
  return jsval;
}

inline JSValue GetBase(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kBaseOffset); }
inline JSValue GetReferencedName(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kReferenceNameOffset); }
inline bool IsStrictReference(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kStrictReferenceOffset, bool); }
inline bool HasPrimitiveBase(JSValue jsval) {
  JSValue base = GetBase(jsval);
  return base.IsBool() || base.IsString() || base.IsNumber();
}
inline bool IsPropertyReference(JSValue jsval) {
  return GetBase(jsval).IsObject() || HasPrimitiveBase(jsval);
}
inline bool IsUnresolvableReference(JSValue jsval) { return GetBase(jsval).IsUndefined(); }

}  // namespace reference

JSValue GetValue(JSValue& e, JSValue V);
void PutValue(JSValue& e, JSValue V, JSValue W);

}  // namespace es

#endif