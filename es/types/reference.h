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
  static Handle<Reference> New(size_t ref_id) {
    return Handle<Reference>(reinterpret_cast<Reference*>((ref_id << STACK_SHIFT) | JS_REF));
  }

  size_t id() { return reinterpret_cast<uint64_t>(this) >> STACK_SHIFT; }

  static bool HasPrimitiveBase(Handle<JSValue> base) {
    Type base_type = base.val()->type();
    return IsBool(base_type) || IsString(base_type) || IsNumber(base_type);
  }
  static bool IsPropertyReference(Handle<JSValue> base) { return base.val()->IsObject() || HasPrimitiveBase(base); }
  static bool IsUnresolvableReference(Handle<JSValue> base) { return base.val()->IsUndefined(); }

 private:
  
};

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V);
void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W);
Handle<JSValue> GetValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict);
void PutValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict, Handle<JSValue> value);

}  // namespace es

#endif