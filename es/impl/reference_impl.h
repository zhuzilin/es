#ifndef ES_IMPL_REFERENCE_IMPL
#define ES_IMPL_REFERENCE_IMPL

#include <es/types.h>

namespace es {

Handle<JSValue> GetValue(Handle<Error>& e, Handle<JSValue> V) {
  if (!(V.val()->IsReference())) {
    return V;
  }
  Handle<Reference> ref = static_cast<Handle<Reference>>(V);
  auto stack_ref = Runtime::TopContext().GetReference(ref.val()->id());
  Handle<JSValue> base = stack_ref.base;
  Handle<String> name = stack_ref.name;
  TEST_LOG("GetValue: " + base.ToString() + "." + name.ToString());

  if (Reference::IsUnresolvableReference(base)) {
    e = Error::ReferenceError(name.val()->data() + u" is not defined");
    return Handle<JSValue>();
  }
  if (Reference::IsPropertyReference(base)) {  // 4
    // 4.a & 4.b
    if (base.val()->IsObject()) {
      Handle<JSObject> obj = static_cast<Handle<JSObject>>(base);
      return Get(e, obj, name);
    } else {  // special [[Get]]
      Handle<JSObject> O;
      if (base.val()->IsString()) {
        Handle<String> s = base;
        size_t length = s.val()->size();
        if (name.val()->IsArrayIndex()) {
          size_t index = name.val()->Index();
          if (index < length) {
            return String::Substr(s, index, 1);
          } else {
            return Undefined::Instance();
          }
        } else if (StringEqual(name, String::Length())) {
          return Number::New(length);
        } else {
          O = StringProto::Instance();
        }
      } else {
        O = ToObject(e, base);
      }
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      StackPropertyDescriptor desc = GetProperty(O, name);
      if (desc.IsUndefined())
        return Undefined::Instance();
      if (desc.IsDataDescriptor()) {
        return desc.Value();
      } else {
        ASSERT(desc.IsAccessorDescriptor());
        Handle<JSValue> getter = desc.Get();
        if (getter.val()->IsUndefined()) {
          return Undefined::Instance();
        }
        Handle<JSObject> getter_obj = static_cast<Handle<JSObject>>(getter);
        return Call(e, getter_obj, base, {});
      }
    }
  } else {
    ASSERT(base.val()->IsEnvironmentRecord());
    bool is_strict = Runtime::TopContext().strict();
    Handle<EnvironmentRecord> er = static_cast<Handle<EnvironmentRecord>>(base);
    return GetBindingValue(e, er, name, is_strict);
  }
}

void PutValue(Handle<Error>& e, Handle<JSValue> V, Handle<JSValue> W) {
  if (!V.val()->IsReference()) {
    e = Error::ReferenceError(u"put value to non-reference.");
    return;
  }
  Handle<Reference> ref = static_cast<Handle<Reference>>(V);
  auto stack_ref = Runtime::TopContext().GetReference(ref.val()->id());
  Handle<JSValue> base = stack_ref.base;
  Handle<String> name = stack_ref.name;
  TEST_LOG("PutValue: " + base.ToString() + "." + name.ToString() + " = " + W.ToString());
  bool is_strict = Runtime::TopContext().strict();

  if (Reference::IsUnresolvableReference(base)) {  // 3
    if (is_strict) {  // 3.a
      e = Error::ReferenceError(name.val()->data() + u" is not defined");
      return;
    }
    Put(e, GlobalObject::Instance(), name, W, false);  // 3.b
  } else if (Reference::IsPropertyReference(base)) {
    if (!Reference::HasPrimitiveBase(base)) {
      ASSERT(base.val()->IsObject());
      Handle<JSObject> base_obj = static_cast<Handle<JSObject>>(base);
      Put(e, base_obj, name, W, is_strict);
    } else {  // special [[Put]]
      Handle<JSObject> O = ToObject(e, base);
      if (!CanPut(O, name)) {  // 2
        if (is_strict)
          e = Error::TypeError();
        return;
      }
      StackPropertyDescriptor desc = GetOwnProperty(O, name);  // 3
      if(!desc.IsUndefined()) {
        if (desc.IsDataDescriptor()) {  // 4
          if (is_strict)
            e = Error::TypeError();
          return;
        }
      }
      desc = GetProperty(O, name);
      if (!desc.IsUndefined()) {
        if (desc.IsAccessorDescriptor()) {  // 4
          Handle<JSValue> setter = desc.Set();
          ASSERT(!setter.val()->IsUndefined());
          Handle<JSObject> setter_obj = static_cast<Handle<JSObject>>(setter);
          Call(e, setter_obj, base, {W});
        } else {  // 7
          if (is_strict)
            e = Error::TypeError();
          return;
        }
      }
    }
  } else {
    ASSERT(base.val()->IsEnvironmentRecord());
    Handle<EnvironmentRecord> er = static_cast<Handle<EnvironmentRecord>>(base);
    SetMutableBinding(e, er, name, W, is_strict);
  }
}

Handle<JSValue> GetValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict) {
  if (base.val()->IsUndefined()) {
    e = Error::ReferenceError(name.val()->data() + u" is not defined");
    return Handle<JSValue>();
  } else {
    ASSERT(base.val()->IsEnvironmentRecord());
    Handle<EnvironmentRecord> er = static_cast<Handle<EnvironmentRecord>>(base);
    return GetBindingValue(e, er, name, strict);
  }
}

void PutValueEnvRec(Handle<Error>& e, Handle<JSValue> base, Handle<String> name, bool strict, Handle<JSValue> value) {
  if (base.val()->IsUndefined()) {  // 3
    if (strict) {  // 3.a
      e = Error::ReferenceError(name.val()->data() + u" is not defined");
      return;
    }
    Put(e, GlobalObject::Instance(), name, value, false);  // 3.b
  } else {
    ASSERT(base.val()->IsEnvironmentRecord());
    Handle<EnvironmentRecord> er = static_cast<Handle<EnvironmentRecord>>(base);
    SetMutableBinding(e, er, name, value, strict);
  }
}

}  // namespace es

#endif