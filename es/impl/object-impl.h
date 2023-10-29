#ifndef ES_TYPES_OBJECT_IMPL
#define ES_TYPES_OBJECT_IMPL

#include <es/types.h>

namespace es {

// [[GetOwnProperty]]
Handle<JSValue> GetOwnProperty(Handle<JSObject> O, Handle<String> P) {
  if (O.val()->IsStringObject()) {
    return GetOwnProperty__String(static_cast<Handle<StringObject>>(O), P);
  } else if (O.val()->IsArgumentsObject()) {
    return GetOwnProperty__Arguments(static_cast<Handle<ArgumentsObject>>(O), P);
  } else {
    return GetOwnProperty__Base(O, P);
  }
}

// 8.12.1 [[GetOwnProperty]] (P)
Handle<JSValue> GetOwnProperty__Base(Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> val = O.val()->named_properties()->Get(P);
  if (val.IsNullptr()) {
    return Undefined::Instance();
  }
  // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
  // And assign the property to it. However, if we init D->value = a, and
  // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
  // remain b and that is not what we want.
  return val;
}

// 15.5.5.2 [[GetOwnProperty]] ( P )
Handle<JSValue> GetOwnProperty__String(Handle<StringObject> O, Handle<String> P) {
  Handle<JSValue> val = GetOwnProperty__Base(O, P);
  if (!val.val()->IsUndefined())
    return val;
  Handle<Error> e = Error::Ok();
  if (!P.val()->IsArrayIndex())
    return Undefined::Instance();
  uint32_t index = ToInteger(e, P);  // this will never has error.
  Handle<String> str = static_cast<Handle<String>>(O.val()->PrimitiveValue());
  uint32_t len = str.val()->size();
  if (len <= index)
    return Undefined::Instance();
  Handle<String> substr = String::Substr(str, index, 1);
  Handle<PropertyDescriptor> desc = PropertyDescriptor::NewDataDescriptor(substr, true, false, false);
  return desc;
}

// 10.6
Handle<JSValue> GetOwnProperty__Arguments(Handle<ArgumentsObject> O, Handle<String> P) {
  Handle<JSValue> val = GetOwnProperty__Base(O, P);
  return val;
}

// [[GetProperty]]
// 8.12.2 [[GetProperty]] (P)
Handle<JSValue> GetProperty(Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> own_property = GetOwnProperty(O, P);
  if (!own_property.val()->IsUndefined()) {
    return own_property;
  }
  Handle<JSValue> proto = O.val()->Prototype();
  if (proto.val()->IsNull()) {
    return Undefined::Instance();
  }
  ASSERT(proto.val()->IsObject());
  Handle<JSObject> proto_obj = static_cast<Handle<JSObject>>(proto);
  return GetProperty(proto_obj, P);
}

// [[Get]]
Handle<JSValue> Get(Handle<Error>& e, Handle<JSObject> O, Handle<String> P) {
  TEST_LOG("enter Get " + P.ToString() + " from " + std::to_string(O.val()->type()));
  if (O.val()->IsFunctionObject()) {
    return Get__Function(e, static_cast<Handle<FunctionObject>>(O), P);
  } else if (O.val()->IsArgumentsObject()) {
    return Get__Arguments(e, static_cast<Handle<ArgumentsObject>>(O), P);
  } else {
    return Get__Base(e, O, P);
  }
}

// 8.12.3 [[Get]] (P) 
Handle<JSValue> Get__Base(Handle<Error>& e, Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> value = GetProperty(O, P);
  if (value.val()->IsUndefined()) {
    return Undefined::Instance();
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
  ASSERT(desc.val()->IsPropertyDescriptor());
  if (desc.val()->IsDataDescriptor()) {
    return desc.val()->Value();
  } else {
    ASSERT(desc.val()->IsAccessorDescriptor());
    Handle<JSValue> getter = desc.val()->Get();
    if (getter.val()->IsUndefined())
      return Undefined::Instance();
    return Call(e, getter, O);
  }
}

// 15.3.5.4 [[Get]] (P)
Handle<JSValue> Get__Function(Handle<Error>& e, Handle<FunctionObject> O, Handle<String> P) {
  Handle<JSValue> V = Get__Base(e, O, P);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  if (V.val()->IsFunctionObject() && StringEqual(P, String::caller())) {  // 2
    Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(V);
    if (func.val()->strict()) {
      e = Error::TypeError(u"cannot get caller property from function in strict mode.");
      return Handle<JSValue>();
    }
  }
  return V;
}

// 10.6
Handle<JSValue> Get__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P) {
  if (Runtime::TopContext().strict()) {
    if (StringEqual(P, String::caller()) || StringEqual(P, String::callee())) {
      e = Error::TypeError(u"access callee or caller in strict mode");
      return Handle<JSValue>();
    }
  }
  Handle<JSValue> V = Get__Base(e, O, P);
  return V;
}

// [[CanPut]]
// 8.12.4 [[CanPut]] (P)
bool CanPut(Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> value = GetOwnProperty(O, P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
    if (desc.val()->IsAccessorDescriptor()) {
      return !desc.val()->Set().val()->IsUndefined();
    } else {
      return desc.val()->Writable();
    }
  }

  Handle<JSValue> proto = O.val()->Prototype();
  if (proto.val()->IsNull()) {
    return O.val()->Extensible();
  }
  Handle<JSObject> proto_obj = static_cast<Handle<JSObject>>(proto);
  Handle<JSValue> inherit = GetProperty(proto_obj, P);
  if (inherit.val()->IsUndefined()) {
    return O.val()->Extensible();
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(inherit);
  if (desc.val()->IsAccessorDescriptor()) {
    return !desc.val()->Set().val()->IsUndefined();
  } else {
    return O.val()->Extensible() ? desc.val()->Writable() : false;
  }
}

// [[Put]]
// 8.12.5 [[Put]] ( P, V, Throw )
void Put(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, Handle<JSValue> V, bool throw_flag) {
  TEST_LOG("enter Put " + P.ToString() + " to " + O.ToString() + " with value " + V.ToString());
  ASSERT(V.val()->IsLanguageType());
  if (!CanPut(O, P)) {  // 1
    if (throw_flag) {  // 1.a
      e = Error::TypeError(u"cannot put " + P.val()->data());
    }
    return;  // 1.b
  }
  Handle<JSValue> value = GetOwnProperty(O, P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> own_desc = static_cast<Handle<PropertyDescriptor>>(value);  // 2
    if (own_desc.val()->IsDataDescriptor()) {  // 3
      Handle<PropertyDescriptor> value_desc = PropertyDescriptor::New();
      value_desc.val()->SetValue(V);
      if (unlikely(log::Debugger::On()))
        log::PrintSource("Overwrite the old desc with " + value_desc.ToString());
      DefineOwnProperty(e, O, P, value_desc, throw_flag);
      return;
    }
  } else {
    // expand GetProperty to prevent another GetOwnProperty(O, P)
    Handle<JSValue> proto = O.val()->Prototype();
    if (!proto.val()->IsNull()) {
      ASSERT(proto.val()->IsObject());
      value = GetProperty(static_cast<Handle<JSObject>>(proto), P);
    }
  }
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
    if (desc.val()->IsAccessorDescriptor()) {
      if (unlikely(log::Debugger::On()))
        log::PrintSource("Use parent prototype's setter");
      Handle<JSValue> setter = desc.val()->Set();
      ASSERT(!setter.val()->IsUndefined());
      Call(e, setter, O, {V});
      return;
    }
  }
  Handle<PropertyDescriptor> new_desc = PropertyDescriptor::NewDataDescriptor(
    V, true, true, true);  // 6.a
  DefineOwnProperty(e, O, P, new_desc, throw_flag);
}

// [[HasProperty]]
// 8.12.6 [[HasProperty]] (P)
bool HasProperty(Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> desc = GetProperty(O, P);
  return !desc.val()->IsUndefined();
}

// [[Delete]]
bool Delete(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, bool throw_flag) {
  if (O.val()->IsArgumentsObject()) {
    return Delete__Arguments(e, static_cast<Handle<ArgumentsObject>>(O), P, throw_flag);
  } else {
    return Delete__Base(e, O, P, throw_flag);
  }
}

// 8.12.7 [[Delete]] (P, Throw)
bool Delete__Base(Handle<Error>& e, Handle<JSObject> O, Handle<String> P, bool throw_flag) {
  Handle<JSValue> value = GetOwnProperty(O, P);
  if (value.val()->IsUndefined()) {
    return true;
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
  if (desc.val()->Configurable()) {
    O.val()->named_properties()->Delete(P);
    return true;
  } else {
    if (throw_flag) {
      e = Error::TypeError(P.val()->data() + u" not configurable, therefore failed to delete");
    }
    return false;
  }
}

bool Delete__Arguments(Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P, bool throw_flag) {
  bool result = Delete__Base(e, O, P, throw_flag);
  return result;
}

// [[DefaultValue]]
// 8.12.8 [[DefaultValue]] (hint)
Handle<JSValue> DefaultValue(Handle<Error>& e, Handle<JSObject> O, std::u16string hint) {
  Handle<String> first, second;
  if (hint == u"String" || (hint == u"" && O.val()->IsDateObject())) {
    first = String::toString();
    second = String::valueOf();
  } else if (hint == u"Number" || (hint == u"" && !O.val()->IsDateObject())) {
    first = String::valueOf();
    second = String::toString();
  } else {
    assert(false);
  }

  ValueGuard guard;
  guard.AddValue(O);

  Handle<JSValue> to_string = Get(e, O, first);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  if (to_string.val()->IsCallable()) {
    Handle<JSObject> to_string_obj = static_cast<Handle<JSObject>>(to_string);
    Handle<JSValue> str = Call(e, to_string_obj, O);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (str.val()->IsPrimitive()) {
      return str;
    }
  }
  Handle<JSValue> value_of = Get(e, O, second);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  if (value_of.val()->IsCallable()) {
    Handle<JSObject> value_of_obj = static_cast<Handle<JSObject>>(value_of);
    Handle<JSValue> val = Call(e, value_of_obj, O);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (val.val()->IsPrimitive()) {
      return val;
    }
  }
  e = Error::TypeError(u"failed to get [[DefaultValue]]");
  return Handle<JSValue>();
}

// [[DefineOwnProperty]]
bool DefineOwnProperty(
  Handle<Error>& e, Handle<JSObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  TEST_LOG("enter DefineOwnProperty " + P.ToString());
  if (O.val()->IsArrayObject()) {
    return DefineOwnProperty__Array(e, static_cast<Handle<ArrayObject>>(O), P, desc, throw_flag);
  } else if (O.val()->IsArgumentsObject()) {
    return DefineOwnProperty__Arguments(e, static_cast<Handle<ArgumentsObject>>(O), P, desc, throw_flag);
  } else {
    return DefineOwnProperty__Base(e, O, P, desc, throw_flag);
  }
}

bool DefineOwnProperty__Base(
  Handle<Error>& e, Handle<JSObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  Handle<JSValue> current = GetOwnProperty(O, P);
  Handle<PropertyDescriptor> current_desc;
  std::u16string error_msg;
  if (current.val()->IsUndefined()) {
    if(!O.val()->Extensible()) { // 3
      error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": object not extensible";
      goto reject;
    }
    // 4.
    PropertyMap::Set(Handle<PropertyMap>(O.val()->named_properties()), P, desc);
    return true;
  }
  if (desc.val()->bitmask() == 0) {  // 5
    return true;
  }
  current_desc = static_cast<Handle<PropertyDescriptor>>(current);
  if ((desc.val()->bitmask() & current_desc.val()->bitmask()) == desc.val()->bitmask()) {
    if (desc.val()->IsSameAs(current_desc)) return true;  // 6
  }
  if (!current_desc.val()->Configurable()) { // 7
    if (desc.val()->Configurable()) {  // 7.a
      error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": old value not configurable, while new value configurable";
      goto reject;
    }
    if (desc.val()->HasEnumerable() && (desc.val()->Enumerable() != current_desc.val()->Enumerable())) {  // 7.b
      error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": enumerable value differ";
      goto reject;
    }
  }
  // 8.
  if (!desc.val()->IsGenericDescriptor()) {
    if (current_desc.val()->IsDataDescriptor() != desc.val()->IsDataDescriptor()) {  // 9.
      // 9.a
      if (!current_desc.val()->Configurable()) {
        error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": current value not configurable";
        goto reject;
      }
      // 9.b.i & 9.c.i
      current_desc.val()->Reset(
        desc.val()->bitmask(),
        current_desc.val()->Enumerable(),
        current_desc.val()->Configurable());
    } else if (current_desc.val()->IsDataDescriptor() && desc.val()->IsDataDescriptor()) {  // 10.
      if (!current_desc.val()->Configurable()) {  // 10.a
        if (!current_desc.val()->Writable()) {
          if (desc.val()->Writable()) {
            error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": current value not writable";
            goto reject;  // 10.a.i
          }
          // 10.a.ii.1
          if (desc.val()->HasValue() && !SameValue(desc.val()->Value(), current_desc.val()->Value())) {
            error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": current value not writable";
            goto reject;
          }
        }
      } else {  // 10.b
        ASSERT(current_desc.val()->Configurable());
      }
    } else {  // 11.
      ASSERT(current_desc.val()->IsAccessorDescriptor() && desc.val()->IsAccessorDescriptor());
      if (!current_desc.val()->Configurable()) {  // 11.a
        if (!SameValue(desc.val()->Set(), current_desc.val()->Set()) ||  // 11.a.i
            !SameValue(desc.val()->Get(), current_desc.val()->Get())) {  // 11.a.ii
          error_msg = u"failed to DefineOwnProperty " + P.val()->data() + u": different get or set";
          goto reject;
        }
      }
    }
  }
  TEST_LOG("DefineOwnProperty: " + P.ToString() + " is set to " + desc.val()->Value().ToString());
  // 12.
  current_desc.val()->Set(desc);
  // 13.
  return true;
reject:
  TEST_LOG("reject :", error_msg);
  if (throw_flag) {
    e = Error::TypeError(error_msg);
  }
  return false;
}

bool DefineOwnProperty__Array(
  Handle<Error>& e, Handle<ArrayObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  auto old_len_desc = static_cast<Handle<PropertyDescriptor>>(GetOwnProperty(O, String::Length()));
  ASSERT(!old_len_desc.val()->IsUndefined());
  double old_len = ToNumber(e, old_len_desc.val()->Value());
  if (StringEqual(P.val(), String::Length().val())) {  // 3
    if (!desc.val()->HasValue()) {  // 3.a
      return DefineOwnProperty__Base(e, O, String::Length(), desc, throw_flag);
    }
    Handle<PropertyDescriptor> new_len_desc = PropertyDescriptor::New();
    new_len_desc.val()->Set(desc);
    double new_len = ToUint32(e, desc.val()->Value());
    if (unlikely(!e.val()->IsOk())) goto reject;
    double new_num = ToNumber(e, desc.val()->Value());
    if (unlikely(!e.val()->IsOk())) goto reject;
    if (new_len != new_num) {
      e = Error::RangeError(u"length of array need to be uint32.");
      return false;
    }
    Handle<Number> new_len_handle = Number::New(new_len);
    new_len_desc.val()->SetValue(new_len_handle);
    if (new_len >= old_len) {  // 3.f
      return DefineOwnProperty__Base(e, O, String::Length(), new_len_desc, throw_flag);
    }
    if (!old_len_desc.val()->Writable())  // 3.g
      goto reject;
    bool new_writable;
    if (new_len_desc.val()->HasWritable() && new_len_desc.val()->Writable()) {  // 3.h
      new_writable = true;
    } else {  // 3.l
      new_writable = false;
      new_len_desc.val()->SetWritable(true);
    }
    bool succeeded = DefineOwnProperty__Base(e, O, String::Length(), new_len_desc, throw_flag);
    if (!succeeded) return false;  // 3.k
    while (new_len < old_len) {  // 3.l
      old_len--;
      bool delete_succeeded = Delete(e, O, NumberToString(old_len), false);
      if (!delete_succeeded) {  // 3.l.iii
        Handle<Number> new_len_handle = Number::New(old_len + 1);
        new_len_desc.val()->SetValue(new_len_handle);
        if (!new_writable)  // 3.l.iii.2
          new_len_desc.val()->SetWritable(false);
        DefineOwnProperty__Base(e, O, String::Length(), new_len_desc, false);
        goto reject;  // 3.l.iii.4
      }
    }
    if (!new_writable) {  // 3.m
      auto tmp = PropertyDescriptor::New();
      tmp.val()->SetWritable(false);
      ASSERT(DefineOwnProperty__Base(e, O, String::Length(), new_len_desc, false));
      return true;
    }
    return true;  // 3.n
  } else {
    if (P.val()->IsArrayIndex()) {  // 4
      double index = P.val()->Index();
      if (index >= old_len && !old_len_desc.val()->Writable())  // 4.b
        goto reject;
      bool succeeded = DefineOwnProperty__Base(e, O, P, desc, false);
      if (!succeeded)
        goto reject;
      if (index >= old_len) {  // 4.e
        Handle<Number> len_handle = Number::New(index + 1);
        old_len_desc.val()->SetValue(len_handle);
        return DefineOwnProperty__Base(e, O, String::Length(), old_len_desc, false);
      }
      return true;
    }
  }
  return DefineOwnProperty__Base(e, O, P, desc, throw_flag);
reject:
  TEST_LOG("Array::DefineOwnProperty reject " + P.ToString() + " " + desc.ToString());
  if (throw_flag) {
    e = Error::TypeError(u"failed to DefineOwnProperty on Array");
  }
  return false;
}

bool DefineOwnProperty__Arguments(
  Handle<Error>& e, Handle<ArgumentsObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  bool allowed = DefineOwnProperty__Base(e, O, P, desc, throw_flag);
  return allowed;
}

// [[HasInstance]]
bool HasInstance(Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> V) {
  if (O.val()->IsFunctionObject()) {
    Handle<FunctionObject> F = static_cast<Handle<FunctionObject>>(O);
    if (!F.val()->from_bind()) {
      return HasInstance__Function(e, F, V);
    } else {
      return HasInstance__BindFunction(e, Handle<BindFunctionObject>(F), V);
    }
  } else {
    return HasInstance__Base(e, O, V);
  }
}

// NOTE(zhuzilin) Here we use the implementation in 15.3.5.3 [[HasInstance]] (V)
// to make sure all callables have HasInstance.
bool HasInstance__Base(Handle<Error>& e, Handle<JSObject> obj, Handle<JSValue> V) {
  ASSERT(obj.val()->IsCallable());
  if (!V.val()->IsObject())
    return false;
  Handle<JSValue> O = Get(e, obj, String::Prototype());
  if (unlikely(!e.val()->IsOk())) return false;
  if (!O.val()->IsObject()) {
    e = Error::TypeError(u"non-object prototype.");
    return false;
  }
  while (!V.val()->IsNull()) {
    if (V.val() == O.val())
      return true;
    ASSERT(V.val()->IsObject());
    V = static_cast<Handle<JSObject>>(V).val()->Prototype();
    if (unlikely(!e.val()->IsOk())) return false;
  }
  return false; 
}

// 15.3.5.3 [[HasInstance]] (V)
bool HasInstance__Function(Handle<Error>& e, Handle<FunctionObject> obj, Handle<JSValue> V) {
  if (!V.val()->IsObject())
    return false;
  Handle<JSValue> O = Get(e, obj, String::Prototype());
  if (unlikely(!e.val()->IsOk())) return false;
  if (!O.val()->IsObject()) {
    e = Error::TypeError();
    return false;
  }
  while (!V.val()->IsNull()) {
    if (V.val() == O.val())
      return true;
    V = Get(e, static_cast<Handle<JSObject>>(V), String::Prototype());
    if (unlikely(!e.val()->IsOk())) return false;
  }
  return false;
}

// 15.3.4.5.3 [[HasInstance]] (V)
bool HasInstance__BindFunction(Handle<Error>& e, Handle<BindFunctionObject> obj, Handle<JSValue> V) {
  return HasInstance(e, obj.val()->TargetFunction(), V);
}

void AddValueProperty(
  Handle<JSObject> O, Handle<String> name, Handle<JSValue> value, bool writable,
  bool enumerable, bool configurable
) {
  TEST_LOG("AddValueProperty " + O.ToString() + "." + name.ToString() + " = " + value.ToString());
  Handle<PropertyDescriptor> desc = PropertyDescriptor::NewDataDescriptor(
    value, writable, enumerable, configurable);
  // This should just like named_properties_[name] = desc
  ASSERT(GetOwnProperty(O, name).val()->IsUndefined());
  PropertyMap::Set(Handle<PropertyMap>(O.val()->named_properties()), name, desc);
}

}  // namespace es

#endif