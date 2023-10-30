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
  static Handle<Reference> Get(size_t ref_id) {
    if (ref_id < const_references_.size()) {
      return const_references_[ref_id];
    }
    ASSERT(const_references_.size() == ref_id);
    const_references_.emplace_back(Reference::New(ref_id, GCFlag::CONST));
    return const_references_[ref_id];
  }

  size_t id() { return READ_VALUE(this, kIdOffset, size_t); }

  static bool HasPrimitiveBase(Handle<JSValue> base) {
    Type base_type = base.val()->type();
    return IsBool(base_type) || IsString(base_type) || IsNumber(base_type);
  }
  static bool IsPropertyReference(Handle<JSValue> base) { return base.val()->IsObject() || HasPrimitiveBase(base); }
  static bool IsUnresolvableReference(Handle<JSValue> base) { return base.val()->IsUndefined(); }

 private:
  static Handle<Reference> New(
    size_t ref_id,
    flag_t flag
  ) {
    Handle<JSValue> jsval = HeapObject::New(kReferenceSize - kJSValueOffset, flag);
    SET_VALUE(jsval.val(), kIdOffset, ref_id, size_t);

    jsval.val()->SetType(JS_REF);
    return Handle<Reference>(jsval);
  }
  static std::vector<Handle<Reference>> const_references_;

 public:
  static constexpr size_t kIdOffset = kJSValueOffset;
  static constexpr size_t kReferenceSize = kIdOffset + kSizeTSize;
};

std::vector<Handle<Reference>> Reference::const_references_;

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V);
void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W);
Handle<JSValue> GetValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict);
void PutValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict, Handle<JSValue> value);

}  // namespace es

#endif