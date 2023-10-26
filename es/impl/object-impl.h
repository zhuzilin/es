#ifndef ES_TYPES_OBJECT_IMPL
#define ES_TYPES_OBJECT_IMPL

#include <es/types.h>

namespace es {

// [[GetOwnProperty]]
JSValue GetOwnProperty(JSValue O, JSValue P) {
  ASSERT(O.IsObject() && P.IsString());
  if (O.IsStringObject()) {
    return GetOwnProperty__String(O, P);
  } else if (O.IsArgumentsObject()) {
    return GetOwnProperty__Arguments(O, P);
  } else {
    return GetOwnProperty__Base(O, P);
  }
}

// 8.12.1 [[GetOwnProperty]] (P)
JSValue GetOwnProperty__Base(JSValue O, JSValue P) {
  ASSERT(O.IsObject() && P.IsString());
  JSValue val = hash_map::Get(js_object::named_properties(O), P);
  if (val.IsNull()) {
    return undefined::New();
  }
  // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
  // And assign the property to it. However, if we init D->value = a, and
  // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
  // remain b and that is not what we want.
  return val;
}

// 15.5.5.2 [[GetOwnProperty]] ( P )
JSValue GetOwnProperty__String(JSValue O, JSValue P) {
  ASSERT(O.IsStringObject() && P.IsString());
  JSValue val = GetOwnProperty__Base(O, P);
  if (!val.IsUndefined())
    return val;
  JSValue e = error::Ok();
  int index = ToInteger(e, P);  // this will never has error.
  if (string::data(NumberToString(fabs(index))) != string::data(P))
    return undefined::New();
  std::u16string str = string::data(js_object::PrimitiveValue(O));
  int len = str.size();
  if (len <= index)
    return undefined::New();
  JSValue desc = property_descriptor::New();
  JSValue substr = string::New(str.substr(index, 1));
  property_descriptor::SetDataDescriptor(desc, substr, true, false, false);
  return desc;
}

// 10.6
JSValue GetOwnProperty__Arguments(JSValue O, JSValue P) {
  JSValue val = GetOwnProperty__Base(O, P);
  return val;
}

// [[GetProperty]]
// 8.12.2 [[GetProperty]] (P)
JSValue GetProperty(JSValue O, JSValue P) {
  JSValue own_property = GetOwnProperty(O, P);
  if (!own_property.IsUndefined()) {
    return own_property;
  }
  JSValue proto = js_object::Prototype(O);
  if (proto.IsNull()) {
    return undefined::New();
  }
  ASSERT(proto.IsObject());
  return GetProperty(proto, P);
}

// [[Get]]
JSValue Get(JSValue& e, JSValue O, JSValue P) {
  ASSERT(P.IsString() && O.IsObject());
  TEST_LOG("enter Get " + JSValue::ToString(P) + " from " + std::to_string(O.type()));
  if (O.IsFunctionObject()) {
    return Get__Function(e, O, P);
  } else if (O.IsArgumentsObject()) {
    return Get__Arguments(e, O, P);
  } else {
    return Get__Base(e, O, P);
  }
}

// 8.12.3 [[Get]] (P) 
JSValue Get__Base(JSValue& e, JSValue O, JSValue P) {
  JSValue desc = GetProperty(O, P);
  if (desc.IsUndefined()) {
    return undefined::New();
  }
  ASSERT(desc.IsPropertyDescriptor());
  if (property_descriptor::IsDataDescriptor(desc)) {
    return property_descriptor::Value(desc);
  } else {
    ASSERT(property_descriptor::IsAccessorDescriptor(desc));
    JSValue getter = property_descriptor::Get(desc);
    if (getter.IsUndefined())
      return undefined::New();
    return Call(e, getter, O);
  }
}

// 15.3.5.4 [[Get]] (P)
JSValue Get__Function(JSValue& e, JSValue O, JSValue P) {
  JSValue V = Get__Base(e, O, P);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (string::data(P) == u"caller") {  // 2
    if (V.IsFunctionObject() && function_object::strict(V)) {
      e = error::TypeError(u"cannot get caller property from function in strict mode.");
      return JSValue();
    }
  }
  return V;
}

// 10.6
JSValue Get__Arguments(JSValue& e, JSValue O, JSValue P) {
  if (Runtime::TopContext().strict()) {
    if (string::data(P) == u"callee" || string::data(P) == u"caller") {
      e = error::TypeError(u"access callee or caller in strict mode");
      return JSValue();
    }
  }
  JSValue V = Get__Base(e, O, P);
  return V;
}

// [[CanPut]]
// 8.12.4 [[CanPut]] (P)
bool CanPut(JSValue O, JSValue P) {
  JSValue desc = GetOwnProperty(O, P);
  if (!desc.IsUndefined()) {
    ASSERT(desc.IsPropertyDescriptor());
    if (property_descriptor::IsAccessorDescriptor(desc)) {
      return !property_descriptor::Set(desc).IsUndefined();
    } else {
      return property_descriptor::Writable(desc);
    }
  }

  JSValue proto = js_object::Prototype(O);
  if (proto.IsNull()) {
    return js_object::Extensible(O);
  }
  ASSERT(proto.IsObject());
  JSValue inherit_desc = GetProperty(proto, P);
  if (inherit_desc.IsUndefined()) {
    return js_object::Extensible(O);
  }
  ASSERT(inherit_desc.IsPropertyDescriptor());
  if (property_descriptor::IsAccessorDescriptor(inherit_desc)) {
    return !property_descriptor::Set(inherit_desc).IsUndefined();
  } else {
    return js_object::Extensible(O) ? property_descriptor::Writable(inherit_desc) : false;
  }
}

// [[Put]]
// 8.12.5 [[Put]] ( P, V, Throw )
void Put(JSValue& e, JSValue O, JSValue P, JSValue V, bool throw_flag) {
  TEST_LOG("enter Put " + JSValue::ToString(P) + " to " + JSValue::ToString(O) + " with value " + JSValue::ToString(V));
  ASSERT(V.IsLanguageType());
  if (!CanPut(O, P)) {  // 1
    if (throw_flag) {  // 1.a
      e = error::TypeError(u"cannot put " + string::data(P));
    }
    return;  // 1.b
  }
  JSValue desc = GetOwnProperty(O, P);
  if (!desc.IsUndefined()) {
    ASSERT(desc.IsPropertyDescriptor());  // 2
    if (property_descriptor::IsDataDescriptor(desc)) {  // 3
      JSValue value_desc = property_descriptor::New();
      property_descriptor::SetValue(value_desc, V);
      if (unlikely(log::Debugger::On()))
        log::PrintSource("Overwrite the old desc with " + JSValue::ToString(value_desc));
      DefineOwnProperty(e, O, P, value_desc, throw_flag);
      return;
    }
  } else {
    // expand GetProperty to prevent another GetOwnProperty(O, P)
    JSValue proto = js_object::Prototype(O);
    if (!proto.IsNull()) {
      ASSERT(proto.IsObject());
      desc = GetProperty(proto, P);
    }
  }
  if (!desc.IsUndefined()) {
    ASSERT(desc.IsPropertyDescriptor());
    if (property_descriptor::IsAccessorDescriptor(desc)) {
      if (unlikely(log::Debugger::On()))
        log::PrintSource("Use parent prototype's setter");
      JSValue setter = property_descriptor::Set(desc);
      ASSERT(!setter.IsUndefined());
      Call(e, setter, O, {V});
      return;
    }
  }
  JSValue new_desc = property_descriptor::New();
  property_descriptor::SetDataDescriptor(new_desc, V, true, true, true);  // 6.a
  DefineOwnProperty(e, O, P, new_desc, throw_flag);
}

// [[HasProperty]]
// 8.12.6 [[HasProperty]] (P)
bool HasProperty(JSValue O, JSValue P) {
  JSValue desc = GetProperty(O, P);
  return !desc.IsUndefined();
}

// [[Delete]]
bool Delete(JSValue& e, JSValue O, JSValue P, bool throw_flag) {
  if (O.IsArgumentsObject()) {
    return Delete__Arguments(e, O, P, throw_flag);
  } else {
    return Delete__Base(e, O, P, throw_flag);
  }
}

// 8.12.7 [[Delete]] (P, Throw)
bool Delete__Base(JSValue& e, JSValue O, JSValue P, bool throw_flag) {
  JSValue desc = GetOwnProperty(O, P);
  if (desc.IsUndefined()) {
    return true;
  }
  ASSERT(desc.IsPropertyDescriptor());
  if (property_descriptor::Configurable(desc)) {
    hash_map::Delete(js_object::named_properties(O), P);
    return true;
  } else {
    if (throw_flag) {
      e = error::TypeError(string::data(P) + u" not configurable, therefore failed to delete");
    }
    return false;
  }
}

bool Delete__Arguments(JSValue& e, JSValue O, JSValue P, bool throw_flag) {
  bool result = Delete__Base(e, O, P, throw_flag);
  return result;
}

// [[DefaultValue]]
// 8.12.8 [[DefaultValue]] (hint)
JSValue DefaultValue(JSValue& e, JSValue O, std::u16string hint) {
  JSValue first, second;
  if (hint == u"String" || (hint == u"" && O.IsDateObject())) {
    first = string::New(u"toString");
    second = string::New(u"valueOf");
  } else if (hint == u"Number" || (hint == u"" && !O.IsDateObject())) {
    first = string::New(u"valueOf");
    second = string::New(u"toString");
  } else {
    assert(false);
  }

  ValueGuard guard;
  guard.AddValue(O);

  JSValue to_string = Get(e, O, first);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (to_string.IsCallable()) {
    JSValue str = Call(e, to_string, O);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (str.IsPrimitive()) {
      return str;
    }
  }
  JSValue value_of = Get(e, O, second);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (value_of.IsCallable()) {
    JSValue val = Call(e, value_of, O);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (val.IsPrimitive()) {
      return val;
    }
  }
  e = error::TypeError(u"failed to get [[DefaultValue]]");
  return JSValue();
}

// [[DefineOwnProperty]]
bool DefineOwnProperty(
  JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag
) {
  TEST_LOG("enter DefineOwnProperty " + JSValue::ToString(P));
  if (O.IsArrayObject()) {
    return DefineOwnProperty__Array(e, O, P, desc, throw_flag);
  } else if (O.IsArgumentsObject()) {
    return DefineOwnProperty__Arguments(e, O, P, desc, throw_flag);
  } else {
    return DefineOwnProperty__Base(e, O, P, desc, throw_flag);
  }
}

bool DefineOwnProperty__Base(
  JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag
) {
  JSValue current = GetOwnProperty(O, P);
  std::u16string error_msg;
  if (current.IsUndefined()) {
    if(!js_object::Extensible(O)) { // 3
      error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": object not extensible";
      goto reject;
    }
    // 4.
    JSValue new_named_properties = hash_map::Set(js_object::named_properties(O), P, desc);
    js_object::SetNamedProperties(O, new_named_properties);
    return true;
  }
  if (property_descriptor::bitmask(desc) == 0) {  // 5
    return true;
  }
  ASSERT(current.IsPropertyDescriptor());
  if ((property_descriptor::bitmask(desc) & property_descriptor::bitmask(current)) ==
      property_descriptor::bitmask(desc)) {
    if (property_descriptor::IsSameAs(desc, current)) return true;  // 6
  }
  if (!property_descriptor::Configurable(current)) { // 7
    if (property_descriptor::Configurable(desc)) {  // 7.a
      error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": old value not configurable, while new value configurable";
      goto reject;
    }
    if (property_descriptor::HasEnumerable(desc) &&
        (property_descriptor::Enumerable(desc) != property_descriptor::Enumerable(current))) {  // 7.b
      error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": enumerable value differ";
      goto reject;
    }
  }
  // 8.
  if (!property_descriptor::IsGenericDescriptor(desc)) {
    if (property_descriptor::IsDataDescriptor(current) != property_descriptor::IsDataDescriptor(desc)) {  // 9.
      // 9.a
      if (!property_descriptor::Configurable(current)) {
        error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": current value not configurable";
        goto reject;
      }
      // 9.b.i & 9.c.i
      property_descriptor::Reset(
        current,
        property_descriptor::bitmask(desc),
        property_descriptor::Enumerable(current),
        property_descriptor::Configurable(current));
    } else if (property_descriptor::IsDataDescriptor(current) &&
               property_descriptor::IsDataDescriptor(desc)) {  // 10.
      if (!property_descriptor::Configurable(current)) {  // 10.a
        if (!property_descriptor::Writable(current)) {
          if (property_descriptor::Writable(desc)) {
            error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": current value not writable";
            goto reject;  // 10.a.i
          }
          // 10.a.ii.1
          if (property_descriptor::HasValue(desc) &&
              !SameValue(property_descriptor::Value(desc),
                         property_descriptor::Value(current))) {
            error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": current value not writable";
            goto reject;
          }
        }
      } else {  // 10.b
        ASSERT(property_descriptor::Configurable(current));
      }
    } else {  // 11.
      ASSERT(property_descriptor::IsAccessorDescriptor(current) &&
             property_descriptor::IsAccessorDescriptor(desc));
      if (!property_descriptor::Configurable(current)) {  // 11.a
        if (!SameValue(property_descriptor::Set(desc), property_descriptor::Set(current)) ||  // 11.a.i
            !SameValue(property_descriptor::Get(desc), property_descriptor::Get(current))) {  // 11.a.ii
          error_msg = u"failed to DefineOwnProperty " + string::data(P) + u": different get or set";
          goto reject;
        }
      }
    }
  }
  TEST_LOG("DefineOwnProperty: " + JSValue::ToString(P) + " is set to " + JSValue::ToString(property_descriptor::Value(desc)));
  // 12.
  property_descriptor::Set(current, desc);
  // 13.
  return true;
reject:
  TEST_LOG("reject :", error_msg);
  if (throw_flag) {
    e = error::TypeError(error_msg);
  }
  return false;
}

bool DefineOwnProperty__Array(
  JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag
) {
  JSValue old_len_desc = GetOwnProperty(O, string::Length());
  ASSERT(!old_len_desc.IsUndefined());
  double old_len = ToNumber(e, property_descriptor::Value(old_len_desc));
  if (string::data(P) == string::data(string::Length())) {  // 3
    if (!property_descriptor::HasValue(desc)) {  // 3.a
      return DefineOwnProperty__Base(e, O, string::Length(), desc, throw_flag);
    }
    JSValue new_len_desc = property_descriptor::New();
    property_descriptor::Set(new_len_desc, desc);
    double new_len = ToUint32(e, property_descriptor::Value(desc));
    if (unlikely(!error::IsOk(e))) goto reject;
    double new_num = ToNumber(e, property_descriptor::Value(desc));
    if (unlikely(!error::IsOk(e))) goto reject;
    if (new_len != new_num) {
      e = error::RangeError(u"length of array need to be uint32.");
      return false;
    }
    JSValue new_len_handle = number::New(new_len);
    property_descriptor::SetValue(new_len_desc, new_len_handle);
    if (new_len >= old_len) {  // 3.f
      return DefineOwnProperty__Base(e, O, string::Length(), new_len_desc, throw_flag);
    }
    if (!property_descriptor::Writable(old_len_desc))  // 3.g
      goto reject;
    bool new_writable;
    if (property_descriptor::HasWritable(new_len_desc) && property_descriptor::Writable(new_len_desc)) {  // 3.h
      new_writable = true;
    } else {  // 3.l
      new_writable = false;
      property_descriptor::SetWritable(new_len_desc, true);
    }
    bool succeeded = DefineOwnProperty__Base(e, O, string::Length(), new_len_desc, throw_flag);
    if (!succeeded) return false;  // 3.k
    while (new_len < old_len) {  // 3.l
      old_len--;
      bool delete_succeeded = Delete(e, O, NumberToString(old_len), false);
      if (!delete_succeeded) {  // 3.l.iii
        JSValue new_len_handle = number::New(old_len + 1);
        property_descriptor::SetValue(new_len_desc, new_len_handle);
        if (!new_writable)  // 3.l.iii.2
          property_descriptor::SetWritable(new_len_desc, false);
        DefineOwnProperty__Base(e, O, string::Length(), new_len_desc, false);
        goto reject;  // 3.l.iii.4
      }
    }
    if (!new_writable) {  // 3.m
      JSValue tmp = property_descriptor::New();
      property_descriptor::SetWritable(tmp, false);
      ASSERT(DefineOwnProperty__Base(e, O, string::Length(), tmp, false));
      return true;
    }
    return true;  // 3.n
  } else {
    if (IsArrayIndex(P)) {  // 4
      double index = StringToNumber(P);
      if (index >= old_len && !property_descriptor::Writable(old_len_desc))  // 4.b
        goto reject;
      bool succeeded = DefineOwnProperty__Base(e, O, P, desc, false);
      if (!succeeded)
        goto reject;
      if (index >= old_len) {  // 4.e
        JSValue len_handle = number::New(index + 1);
        property_descriptor::SetValue(old_len_desc, len_handle);
        return DefineOwnProperty__Base(e, O, string::Length(), old_len_desc, false);
      }
      return true;
    }
  }
  return DefineOwnProperty__Base(e, O, P, desc, throw_flag);
reject:
  TEST_LOG("Array::DefineOwnProperty reject " + JSValue::ToString(P) + " " + JSValue::ToString(desc));
  if (throw_flag) {
    e = error::TypeError(u"failed to DefineOwnProperty on Array");
  }
  return false;
}

bool DefineOwnProperty__Arguments(
  JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag
) {
  bool allowed = DefineOwnProperty__Base(e, O, P, desc, throw_flag);
  return allowed;
}

// [[HasInstance]]
bool HasInstance(JSValue& e, JSValue O, JSValue V) {
  if (O.IsFunctionObject()) {
    if (!function_object::from_bind(O)) {
      return HasInstance__Function(e, O, V);
    } else {
      return HasInstance__BindFunction(e, O, V);
    }
  } else {
    return HasInstance__Base(e, O, V);
  }
}

// NOTE(zhuzilin) Here we use the implementation in 15.3.5.3 [[HasInstance]] (V)
// to make sure all callables have HasInstance.
bool HasInstance__Base(JSValue& e, JSValue obj, JSValue V) {
  ASSERT(obj.IsCallable());
  if (!V.IsObject())
    return false;
  JSValue O = Get(e, obj, string::Prototype());
  if (unlikely(!error::IsOk(e))) return false;
  if (!O.IsObject()) {
    e = error::TypeError(u"non-object prototype.");
    return false;
  }
  while (!V.IsNull()) {
    if (V == O)
      return true;
    ASSERT(V.IsObject());
    V = js_object::Prototype(V);
    if (unlikely(!error::IsOk(e))) return false;
  }
  return false; 
}

// 15.3.5.3 [[HasInstance]] (V)
bool HasInstance__Function(JSValue& e, JSValue obj, JSValue V) {
  if (!V.IsObject())
    return false;
  JSValue O = Get(e, obj, string::Prototype());
  if (unlikely(!error::IsOk(e))) return false;
  if (!O.IsObject()) {
    e = error::TypeError();
    return false;
  }
  while (!V.IsNull()) {
    if (V == O)
      return true;
    V = Get(e, js_object::Prototype(V), string::Prototype());
    if (unlikely(!error::IsOk(e))) return false;
  }
  return false;
}

// 15.3.4.5.3 [[HasInstance]] (V)
bool HasInstance__BindFunction(JSValue& e, JSValue obj, JSValue V) {
  return HasInstance(e, bind_function_object::TargetFunction(obj), V);
}

void AddValueProperty(
  JSValue O, JSValue name, JSValue value, bool writable,
  bool enumerable, bool configurable
) {
  TEST_LOG("AddValueProperty key: (" + JSValue::ToString(name) + ") value: (" + JSValue::ToString(value) + ") to " + JSValue::ToString(O));
  JSValue desc = property_descriptor::New();
  property_descriptor::SetDataDescriptor(desc, value, writable, enumerable, configurable);
  // This should just like named_properties_[name] = desc
  JSValue e = error::Empty();
  DefineOwnProperty(e, O, name, desc, false);
}

}  // namespace es

#endif