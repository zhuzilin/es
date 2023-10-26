#ifndef ES_IMPL_REFERENCE_IMPL
#define ES_IMPL_REFERENCE_IMPL

#include <es/types.h>

namespace es {

JSValue GetValue(JSValue& e, JSValue V) {
  if (!V.IsReference()) {
    return V;
  }
  TEST_LOG("GetValue V:" + JSValue::ToString(V));
  if (reference::IsUnresolvableReference(V)) {
    e = error::ReferenceError(string::data(reference::GetReferencedName(V)) + u" is not defined");
    return JSValue();
  }
  JSValue base = reference::GetBase(V);
  if (reference::IsPropertyReference(V)) {  // 4
    // 4.a & 4.b
    if (!reference::HasPrimitiveBase(V)) {
      ASSERT(base.IsObject());
      return Get(e, base, reference::GetReferencedName(V));
    } else {  // special [[Get]]
      JSValue O = ToObject(e, base);
      if (unlikely(!error::IsOk(e))) return JSValue();
      JSValue desc = GetProperty(O, reference::GetReferencedName(V));
      if (desc.IsUndefined())
        return undefined::New();
      ASSERT(desc.IsPropertyDescriptor());
      if (property_descriptor::IsDataDescriptor(desc)) {
        return property_descriptor::Value(desc);
      } else {
        ASSERT(property_descriptor::IsAccessorDescriptor(desc));
        JSValue getter = property_descriptor::Get(desc);
        if (getter.IsUndefined()) {
          return undefined::New();
        }
        ASSERT(getter.IsObject());
        return Call(e, getter, base, {});
      }
    }
  } else {
    ASSERT(base.IsEnvironmentRecord());
    return GetBindingValue(e, base, reference::GetReferencedName(V), reference::IsStrictReference(V));
  }
}

void PutValue(JSValue& e, JSValue V, JSValue W) {
  if (!V.IsReference()) {
    e = error::ReferenceError(u"put value to non-reference.");
    return;
  }
  TEST_LOG("PutValue V: " + JSValue::ToString(V) + ", W: " + JSValue::ToString(W));
  JSValue base = reference::GetBase(V);
  if (reference::IsUnresolvableReference(V)) {  // 3
    if (reference::IsStrictReference(V)) {  // 3.a
      e = error::ReferenceError(string::data(reference::GetReferencedName(V)) + u" is not defined");
      return;
    }
    Put(e, global_object::Instance(), reference::GetReferencedName(V), W, false);  // 3.b
  } else if (reference::IsPropertyReference(V)) {
    bool throw_flag = reference::IsStrictReference(V);
    JSValue P = reference::GetReferencedName(V);
    if (!reference::HasPrimitiveBase(V)) {
      ASSERT(base.IsObject());
      Put(e, base, P, W, throw_flag);
    } else {  // special [[Put]]
      JSValue O = ToObject(e, base);
      if (!CanPut(O, P)) {  // 2
        if (throw_flag)
          e = error::TypeError();
        return;
      }
      JSValue desc = GetOwnProperty(O, P);  // 3
      if(!desc.IsUndefined()) {
        ASSERT(desc.IsPropertyDescriptor());
        if (property_descriptor::IsDataDescriptor(desc)) {  // 4
          if (throw_flag)
            e = error::TypeError();
          return;
        }
      }
      desc = GetProperty(O, P);
      if (!desc.IsUndefined()) {
        ASSERT(desc.IsPropertyDescriptor());
        if (property_descriptor::IsAccessorDescriptor(desc)) {  // 4
          JSValue setter = property_descriptor::Set(desc);
          ASSERT(setter.IsObject());
          Call(e, setter, base, {W});
        } else {  // 7
          if (throw_flag)
            e = error::TypeError();
          return;
        }
      }
    }
  } else {
    ASSERT(base.IsEnvironmentRecord());
    SetMutableBinding(e, base, reference::GetReferencedName(V), W, reference::IsStrictReference(V));
  }
}

}  // namespace es

#endif