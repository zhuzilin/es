#ifndef ES_TYPES_OBJECT_IMPL
#define ES_TYPES_OBJECT_IMPL

#include <es/types/object.h>
#include <es/types/builtin/arguments_object.h>
#include <es/types/builtin/array_object.h>
#include <es/types/builtin/bool_object.h>
#include <es/types/builtin/date_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/math_object.h>
#include <es/types/builtin/number_object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/regexp_object.h>
#include <es/types/builtin/string_object.h>

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
  Error* e = Error::Ok();
  int index = ToInteger(e, P);  // this will never has error.
  if (*NumberToString(fabs(index)).val() != *P.val())
    return Undefined::Instance();
  std::u16string str = static_cast<Handle<String>>(O.val()->PrimitiveValue()).val()->data();
  int len = str.size();
  if (len <= index)
    return Undefined::Instance();
  Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
  desc.val()->SetDataDescriptor(String::New(str.substr(index, 1)), true, false, false);
  return desc;
}

// 10.6
Handle<JSValue> GetOwnProperty__Arguments(Handle<ArgumentsObject> O, Handle<String> P) {
  Handle<JSValue> val = GetOwnProperty__Base(O, P);
  if (val.val()->IsUndefined())
    return val;
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(val);
  Handle<JSObject> map = O.val()->ParameterMap();
  Handle<JSValue> is_mapped = GetOwnProperty(map, P);
  if (!is_mapped.val()->IsUndefined()) {  // 5
    desc.val()->SetValue(Get(nullptr, map, P));
  }
  return desc;
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
  assert(proto.val()->IsObject());
  Handle<JSObject> proto_obj = static_cast<Handle<JSObject>>(proto);
  return GetProperty(proto_obj, P);
}

// [[Get]]
Handle<JSValue> Get(Error* e, Handle<JSObject> O, Handle<String> P) {
  if (O.val()->IsFunctionObject()) {
    return Get__Function(e, static_cast<Handle<FunctionObject>>(O), P);
  } else if (O.val()->IsArgumentsObject()) {
    return Get__Arguments(e, static_cast<Handle<ArgumentsObject>>(O), P);
  } else {
    return Get__Base(e, O, P);
  }
}

// 8.12.3 [[Get]] (P) 
Handle<JSValue> Get__Base(Error* e, Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> value = GetProperty(O, P);
  if (value.val()->IsUndefined()) {
    return Undefined::Instance();
  }
  Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
  if (desc.val()->IsDataDescriptor()) {
    return desc.val()->Value();
  } else {
    assert(desc.val()->IsAccessorDescriptor());
    Handle<JSValue> getter = desc.val()->Get();
    if (getter.val()->IsUndefined()) {
      return Undefined::Instance();
    }
    Handle<JSObject> getter_obj = static_cast<Handle<JSObject>>(getter);
    return Call(e, getter_obj, O);
  }
}

// 15.3.5.4 [[Get]] (P)
Handle<JSValue> Get__Function(Error* e, Handle<FunctionObject> O, Handle<String> P) {
  Handle<JSValue> V = Get__Base(e, O, P);
  if (!e->IsOk()) return Handle<JSValue>();
  if (P.val()->data() == u"caller") {  // 2
    if (V.val()->IsObject()) {
      Handle<JSObject> obj = static_cast<Handle<JSObject>>(V);
      if (obj.val()->IsFunction()) {
        Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(V);
        if (func.val()->strict()) {
          *e = *Error::TypeError();
          return Handle<JSValue>();
        }
      }
    }
  }
  return V;
}

// 10.6
Handle<JSValue> Get__Arguments(Error* e, Handle<ArgumentsObject> O, Handle<String> P) {
  Handle<JSObject> map = O.val()->ParameterMap();
  Handle<JSValue> is_mapped = GetOwnProperty(map, P);
  if (is_mapped.val()->IsUndefined()) {  // 3
    Handle<JSValue> V = Get__Base(e, O, P);
    if (!e->IsOk()) return Handle<JSValue>();
    if (P.val()->data() == u"caller") {
      if (V.val()->IsObject()) {
        Handle<JSObject> obj = static_cast<Handle<JSObject>>(V);
        if (obj.val()->IsFunction()) {
          Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(obj);
          if (func.val()->strict()) {
            *e = *Error::TypeError(u"caller could not be function object");
          }
        }
      }
    }
    return V;
  }
  // 4
  return Get(e, map, P);
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
void Put(Error* e, Handle<JSObject> O, Handle<String> P, Handle<JSValue> V, bool throw_flag) {
  assert(V.val()->IsLanguageType());
  if (!CanPut(O, P)) {  // 1
    if (throw_flag) {  // 1.a
      *e = *Error::TypeError();
    }
    return;  // 1.b
  }
  Handle<JSValue> value = GetOwnProperty(O, P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> own_desc = static_cast<Handle<PropertyDescriptor>>(value);  // 2
    if (own_desc.val()->IsDataDescriptor()) {  // 3
      Handle<PropertyDescriptor> value_desc = PropertyDescriptor::New();
      value_desc.val()->SetValue(V);
      log::PrintSource("Overwrite the old desc with " + value_desc.ToString());
      DefineOwnProperty(e, O, P, value_desc, throw_flag);
      return;
    }
  }
  value = GetProperty(O, P);
  if (!value.val()->IsUndefined()) {
    Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(value);
    if (desc.val()->IsAccessorDescriptor()) {
      log::PrintSource("Use parent prototype's setter");
      Handle<JSValue> setter = desc.val()->Set();
      assert(!setter.val()->IsUndefined());
      Handle<JSObject> setter_obj = static_cast<Handle<JSObject>>(setter);
      Call(e, setter_obj, O, {V});
      return;
    }
  }
  Handle<PropertyDescriptor> new_desc = PropertyDescriptor::New();
  new_desc.val()->SetDataDescriptor(V, true, true, true);  // 6.a
  DefineOwnProperty(e, O, P, new_desc, throw_flag);
}

// [[HasProperty]]
// 8.12.6 [[HasProperty]] (P)
bool HasProperty(Handle<JSObject> O, Handle<String> P) {
  Handle<JSValue> desc = GetOwnProperty(O, P);
  return !desc.val()->IsUndefined();
}

// [[Delete]]
bool Delete(Error* e, Handle<JSObject> O, Handle<String> P, bool throw_flag) {
  if (O.val()->IsArgumentsObject()) {
    return Delete__Arguments(e, static_cast<Handle<ArgumentsObject>>(O), P, throw_flag);
  } else {
    return Delete__Base(e, O, P, throw_flag);
  }
}

// 8.12.7 [[Delete]] (P, Throw)
bool Delete__Base(Error* e, Handle<JSObject> O, Handle<String> P, bool throw_flag) {
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
      *e = *Error::TypeError();
    }
    return false;
  }
}

bool Delete__Arguments(Error* e, Handle<ArgumentsObject> O, Handle<String> P, bool throw_flag) {
  Handle<JSObject> map = O.val()->ParameterMap();
  Handle<JSValue> is_mapped = GetOwnProperty(map, P);
  bool result = Delete__Base(e, O, P, throw_flag);
  if (!e->IsOk()) return false;
  if (result && !is_mapped.val()->IsUndefined()) {
    Delete(e, map, P, false);
  }
  return result;
}

// [[DefaultValue]]
// 8.12.8 [[DefaultValue]] (hint)
Handle<JSValue> DefaultValue(Error* e, Handle<JSObject> O, std::u16string hint) {
  Handle<String> first, second;
  if (hint == u"String" || hint == u"" && O.val()->IsDateObject()) {
    first = String::New(u"toString");
    second = String::New(u"valueOf");
  } else if (hint == u"Number" || hint == u"" && !O.val()->IsDateObject()) {
    first = String::New(u"valueOf");
    second = String::New(u"toString");
  } else {
    assert(false);
  }

  ValueGuard guard;
  guard.AddValue(O);

  Handle<JSValue> to_string = Get(e, O, first);
  if (!e->IsOk()) return Handle<JSValue>();
  if (to_string.val()->IsCallable()) {
    Handle<JSObject> to_string_obj = static_cast<Handle<JSObject>>(to_string);
    Handle<JSValue> str = Call(e, to_string_obj, O);
    if (!e->IsOk()) return Handle<JSValue>();
    if (str.val()->IsPrimitive()) {
      return str;
    }
  }
  Handle<JSValue> value_of = Get(e, O, second);
  if (!e->IsOk()) return Handle<JSValue>();
  if (value_of.val()->IsCallable()) {
    Handle<JSObject> value_of_obj = static_cast<Handle<JSObject>>(value_of);
    Handle<JSValue> val = Call(e, value_of_obj, O);
    if (!e->IsOk()) return Handle<JSValue>();
    if (val.val()->IsPrimitive()) {
      return val;
    }
  }
  *e = *Error::TypeError(u"failed to get [[DefaultValue]]");
  return Handle<JSValue>();
}

// [[DefineOwnProperty]]
bool DefineOwnProperty(
  Error* e, Handle<JSObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  if (O.val()->IsArrayObject()) {
    return DefineOwnProperty__Array(e, static_cast<Handle<ArrayObject>>(O), P, desc, throw_flag);
  } else if (O.val()->IsArgumentsObject()) {
    return DefineOwnProperty__Arguments(e, static_cast<Handle<ArgumentsObject>>(O), P, desc, throw_flag);
  } else {
    return DefineOwnProperty__Base(e, O, P, desc, throw_flag);
  }
}

bool DefineOwnProperty__Base(
  Error* e, Handle<JSObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  Handle<JSValue> current = GetOwnProperty(O, P);
  Handle<PropertyDescriptor> current_desc;
  if (current.val()->IsUndefined()) {
    if(!O.val()->Extensible())  // 3
      goto reject;
    // 4.
    HashMap<PropertyDescriptor>::Set(
      Handle<HashMap<PropertyDescriptor>>(O.val()->named_properties()), P, desc);
    return true;
  }
  if (desc.val()->bitmask() == 0) {  // 5
    return true;
  }
  current_desc = static_cast<Handle<PropertyDescriptor>>(current);
  if ((desc.val()->bitmask() & current_desc.val()->bitmask()) == desc.val()->bitmask()) {
    bool same = true;
    if (desc.val()->HasValue())
      same = same && SameValue(desc.val()->Value(), current_desc.val()->Value());
    if (desc.val()->HasWritable())
      same = same && (desc.val()->Writable() == current_desc.val()->Writable());
    if (desc.val()->HasGet())
      same = same && SameValue(desc.val()->Get(), current_desc.val()->Get());
    if (desc.val()->HasSet())
      same = same && SameValue(desc.val()->Set(), current_desc.val()->Set());
    if (desc.val()->HasConfigurable())
      same = same && (desc.val()->Configurable() == current_desc.val()->Configurable());
    if (desc.val()->HasEnumerable())
      same = same && (desc.val()->Enumerable() == current_desc.val()->Enumerable());
    if (same) return true;  // 6
  }
  log::PrintSource("desc: " + desc.ToString() + ", current: " + current_desc.ToString());
  if (!current_desc.val()->Configurable()) { // 7
    if (desc.val()->Configurable()) {  // 7.a
      log::PrintSource("DefineOwnProperty: " + P.ToString() + " not configurable, while new value configurable");
      goto reject;
    }
    if (desc.val()->HasEnumerable() && (desc.val()->Enumerable() != current_desc.val()->Enumerable())) {  // 7.b
      log::PrintSource("DefineOwnProperty: " + P.ToString() + " enumerable value differ");
      goto reject;
    }
  }
  // 8.
  if (!desc.val()->IsGenericDescriptor()) {
    if (current_desc.val()->IsDataDescriptor() != desc.val()->IsDataDescriptor()) {  // 9.
      // 9.a
      if (!current_desc.val()->Configurable()) goto reject;
      // 9.b.i & 9.c.i
      Handle<PropertyDescriptor> old_property = O.val()->named_properties()->Get(P);
      Handle<PropertyDescriptor> new_property = PropertyDescriptor::New();
      new_property.val()->SetConfigurable(old_property.val()->Configurable());
      new_property.val()->SetEnumerable(old_property.val()->Enumerable());
      new_property.val()->SetBitMask(old_property.val()->bitmask());
      HashMap<PropertyDescriptor>::Set(
        Handle<HashMap<PropertyDescriptor>>(O.val()->named_properties()), P, desc);
    } else if (current_desc.val()->IsDataDescriptor() && desc.val()->IsDataDescriptor()) {  // 10.
      if (!current_desc.val()->Configurable()) {  // 10.a
        if (!current_desc.val()->Writable()) {
          if (desc.val()->Writable()) goto reject;  // 10.a.i
          // 10.a.ii.1
          if (desc.val()->HasValue() && !SameValue(desc.val()->Value(), current_desc.val()->Value())) goto reject;
        }
      } else {  // 10.b
        assert(current_desc.val()->Configurable());
      }
    } else {  // 11.
      assert(current_desc.val()->IsAccessorDescriptor() && desc.val()->IsAccessorDescriptor());
      if (!current_desc.val()->Configurable()) {  // 11.a
        if (!SameValue(desc.val()->Set(), current_desc.val()->Set()) ||  // 11.a.i
            !SameValue(desc.val()->Get(), current_desc.val()->Get()))    // 11.a.ii
          goto reject;
      }
    }
  }
  log::PrintSource("DefineOwnProperty: " + P.ToString() + " is set to " + desc.val()->Value().ToString());
  // 12.
  current_desc.val()->Set(desc);
  // 13.
  return true;
reject:
  log::PrintSource("DefineOwnProperty reject");
  if (throw_flag) {
    *e = *Error::TypeError();
  }
  return false;
}

bool DefineOwnProperty__Array(
  Error* e, Handle<ArrayObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  auto old_len_desc = static_cast<Handle<PropertyDescriptor>>(GetOwnProperty(O, String::Length()));
  assert(!old_len_desc.val()->IsUndefined());
  double old_len = ToNumber(e, old_len_desc.val()->Value());
  if (*P.val() == *String::Length().val()) {  // 3
    if (!desc.val()->HasValue()) {  // 3.a
      return DefineOwnProperty__Base(e, O, String::Length(), desc, throw_flag);
    }
    Handle<PropertyDescriptor> new_len_desc = PropertyDescriptor::New();
    new_len_desc.val()->Set(desc);
    double new_len = ToUint32(e, desc.val()->Value());
    if (!e->IsOk()) goto reject;
    double new_num = ToNumber(e, desc.val()->Value());
    if (!e->IsOk()) goto reject;
    if (new_len != new_num) {
      *e = *Error::RangeError(u"length of array need to be uint32.");
      return false;
    }
    new_len_desc.val()->SetValue(Number::New(new_len));
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
      bool delete_succeeded = Delete(e, O, ::es::ToString(e, Number::New(old_len)), false);
      if (!delete_succeeded) {  // 3.l.iii
        new_len_desc.val()->SetValue(Number::New(old_len + 1));
        if (!new_writable)  // 3.l.iii.2
          new_len_desc.val()->SetWritable(false);
        DefineOwnProperty__Base(e, O, String::Length(), new_len_desc, false);
        goto reject;  // 3.l.iii.4
      }
    }
    if (!new_writable) {  // 3.m
      auto tmp = PropertyDescriptor::New();
      tmp.val()->SetWritable(false);
      assert(DefineOwnProperty__Base(e, O, String::Length(), new_len_desc, false));
      return true;
    }
    return true;  // 3.n
  } else {
    if (IsArrayIndex(P)) {  // 4
      double index = StringToNumber(P);
      if (index >= old_len && !old_len_desc.val()->Writable())  // 4.b
        goto reject;
      bool succeeded = DefineOwnProperty__Base(e, O, P, desc, false);
      if (!succeeded)
        goto reject;
      if (index >= old_len) {  // 4.e
        old_len_desc.val()->SetValue(Number::New(index + 1));
        return DefineOwnProperty__Base(e, O, String::Length(), old_len_desc, false);
      }
      return true;
    }
  }
  return DefineOwnProperty__Base(e, O, P, desc, throw_flag);
reject:
  log::PrintSource("Array::DefineOwnProperty reject " + P.ToString() + " " + desc.ToString());
  if (throw_flag) {
    *e = *Error::TypeError();
  }
  return false;
}

bool DefineOwnProperty__Arguments(
  Error* e, Handle<ArgumentsObject> O, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag
) {
  Handle<JSObject> map = O.val()->ParameterMap();
  Handle<JSValue> is_mapped = GetOwnProperty(map, P);
  bool allowed = DefineOwnProperty__Base(e, O, P, desc, false);
  if (!allowed) {
    if (throw_flag) {
      *e = *Error::TypeError(u"DefineOwnProperty " + P.val()->data() + u" failed");
    }
    return false;
  }
  if (!is_mapped.val()->IsUndefined()) {  // 5
    if (desc.val()->IsAccessorDescriptor()) {
      Delete(e, map, P, false);
    } else {
      if (desc.val()->HasValue()) {
        Put(e, map, P, desc.val()->Value(), false);
      }
      if (desc.val()->HasWritable() && !desc.val()->Writable()) {
        Delete(e, map, P, false);
      }
    }
  }
  return true;
}

// [[HasInstance]]
bool HasInstance(Error* e, Handle<JSObject> O, Handle<JSValue> V) {
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
bool HasInstance__Base(Error* e, Handle<JSObject> obj, Handle<JSValue> V) {
  assert(obj.val()->IsCallable());
  if (!V.val()->IsObject())
    return false;
  Handle<JSValue> O = Get(e, obj, String::Prototype());
  if (!e->IsOk()) return false;
  if (!O.val()->IsObject()) {
    *e = *Error::TypeError(u"non-object prototype.");
    return false;
  }
  while (!V.val()->IsNull()) {
    if (V.val() == O.val())
      return true;
    assert(V.val()->IsObject());
    V = static_cast<Handle<JSObject>>(V).val()->Prototype();
    if (!e->IsOk()) return false;
  }
  return false; 
}

// 15.3.5.3 [[HasInstance]] (V)
bool HasInstance__Function(Error* e, Handle<FunctionObject> obj, Handle<JSValue> V) {
  if (!V.val()->IsObject())
    return false;
  Handle<JSValue> O = Get(e, obj, String::Prototype());
  if (!e->IsOk()) return false;
  if (!O.val()->IsObject()) {
    *e = *Error::TypeError();
    return false;
  }
  while (!V.val()->IsNull()) {
    if (V.val() == O.val())
      return true;
    V = Get(e, static_cast<Handle<JSObject>>(V), String::Prototype());
    if (!e->IsOk()) return false;
  }
  return false;
}

// 15.3.4.5.3 [[HasInstance]] (V)
bool HasInstance__BindFunction(Error* e, Handle<BindFunctionObject> obj, Handle<JSValue> V) {
  return HasInstance(e, obj.val()->TargetFunction(), V);
}

void AddValueProperty(
  Handle<JSObject> O, Handle<String> name, Handle<JSValue> value, bool writable,
  bool enumerable, bool configurable
) {
  Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
  desc.val()->SetDataDescriptor(value, writable, enumerable, configurable);
  // This should just like named_properties_[name] = desc
  DefineOwnProperty(nullptr, O, name, desc, false);
}

Handle<JSValue> Call(
  Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (O.val()->IsFunctionObject()) {
    Handle<FunctionObject> F = static_cast<Handle<FunctionObject>>(O);
    if (!F.val()->from_bind()) {
      return Call__Function(e, F, this_arg, arguments);
    } else {
      return Call__BindFunction(e, static_cast<Handle<BindFunctionObject>>(O), this_arg, arguments);
    }
  } else if (O.val()->IsConstructor()) {
    if (O.val()->IsBoolConstructor()) {
      return Call__BoolConstructor(e, static_cast<Handle<BoolConstructor>>(O), this_arg, arguments);
    } else if (O.val()->IsNumberConstructor()) {
      return Call__NumberConstructor(e, static_cast<Handle<NumberConstructor>>(O), this_arg, arguments);
    } else if (O.val()->IsObjectConstructor()) {
      return Call__ObjectConstructor(e, static_cast<Handle<ObjectConstructor>>(O), this_arg, arguments);
    } else if (O.val()->IsRegExpConstructor()) {
      return Call__RegExpConstructor(e, static_cast<Handle<RegExpConstructor>>(O), this_arg, arguments);
    } else if (O.val()->IsStringConstructor()) {
      return Call__StringConstructor(e, static_cast<Handle<StringConstructor>>(O), this_arg, arguments);
    } else {
      // 15.3.1.1 Function (p1, p2, … , pn, body)
      // 15.5.1.1 Array ( [ value ] )
      // 15.9.2.1 Date ( [ year [, month [, date [, hours [, minutes [, seconds [, ms ] ] ] ] ] ] ] )
      // 15.11.1.1 Error (message)
      return Call__Construct(e, O, this_arg, arguments);
    }
  } else if (O.val()->IsFunctionProto()) {
    return Call__FunctionProto(e, static_cast<Handle<FunctionProto>>(O), this_arg, arguments);
  } else {
    return Call__Base(e, O, this_arg, arguments);
  }
}

Handle<JSValue> Call__Base(
  Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  inner_func callable = O.val()->callable();
  assert(O.val()->IsCallable() && callable != nullptr);
  return callable(e, O, arguments);
}

Handle<JSValue> Call__Construct(
  Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  return O.val()->Construct(e, arguments);
}

// 13.2.1 [[Call]]
Handle<JSValue> Call__Function(
  Error* e, Handle<FunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  ProgramOrFunctionBody* code = O.val()->Code();
  log::PrintSource("enter FunctionObject::Call ", code->source().substr(0, 100));
  EnterFunctionCode(e, O, code, this_arg, arguments, O.val()->strict());
  if (!e->IsOk()) return Handle<JSValue>();

  Completion result;
  if (code != nullptr) {
    result = EvalProgram(code);
  }
  Runtime::Global()->PopContext();   // 3

  log::PrintSource("exit FunctionObject::Call", code->source().substr(0, 100));
  switch (result.type()) {
    case Completion::RETURN:
      log::PrintSource("exit FunctionObject::Call RETURN");
      return result.value();
    case Completion::THROW: {
      log::PrintSource("exit FunctionObject::Call THROW");
      Handle<JSValue> throw_value = result.value();
      if (throw_value.val()->IsErrorObject()) {
        *e = *(static_cast<Handle<ErrorObject>>(throw_value).val()->e());
        log::PrintSource("message: ", e->message());
        return Handle<JSValue>();
      }
      std::u16string message = ToU16String(e, throw_value);
      log::PrintSource("message: ", message);
      *e = *Error::NativeError(message);
      return Handle<JSValue>();
    }
    default:
      log::PrintSource("exit FunctionObject::Call NORMAL");
      assert(result.type() == Completion::NORMAL);
      return Undefined::Instance();
  }
}

Handle<JSValue> Call__BindFunction(
  Error* e, Handle<BindFunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> extra_args
) {
  log::PrintSource("enter BindFunctionObject::Call");
  Handle<FixedArray<JSValue>> bound_args = O.val()->BoundArgs();
  Handle<JSObject> target_function = O.val()->TargetFunction();

  std::vector<Handle<JSValue>> args;
  for (size_t i = 0; i < bound_args.val()->size(); i++) {
    args.emplace_back(bound_args.val()->Get(i));
  }
  args.insert(args.end(), extra_args.begin(), extra_args.end());
  return Call(e, target_function, O.val()->BoundThis(), args);
}

// 15.3.4 The Function prototype object is itself a Function object (its [[Class]] is "Function") that,
// when invoked, accepts any arguments and returns undefined.
Handle<JSValue> Call__FunctionProto(
  Error* e, Handle<FunctionProto> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  return Undefined::Instance();
}

// 15.6.1.1 Boolean (value)
Handle<JSValue> Call__BoolConstructor(
  Error* e, Handle<BoolConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  bool b;
  if (arguments.size() == 0)
    b = ToBoolean(Undefined::Instance());
  else
    b = ToBoolean(arguments[0]);
  return Bool::Wrap(b);
}

// 15.7.1.1 Number ( [ value ] )
Handle<JSValue> Call__NumberConstructor(
  Error* e, Handle<NumberConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  Handle<Number> js_num;
  if (arguments.size() == 0) {
    js_num = Number::Zero();
  } else {
    double num = ToNumber(e, arguments[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    js_num = Number::New(num);
  }
  return js_num;
}

// 15.2.1.1 Object ( [ value ] )
Handle<JSValue> Call__ObjectConstructor(
  Error* e, Handle<ObjectConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0 || arguments[0].val()->IsNull() || arguments[0].val()->IsUndefined())
    return O.val()->Construct(e, arguments);
  return ToObject(e, arguments[0]);
}

// 15.10.4.1 new RegExp(pattern, flags)
Handle<JSValue> Call__RegExpConstructor(
  Error* e, Handle<RegExpConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0) {
    *e = *Error::TypeError(u"RegExp called with 0 parameters");
    return Handle<JSValue>();
  }
  if ((arguments.size() == 1 || arguments[1].val()->IsUndefined()) && arguments[0].val()->IsRegExpObject()) {
      return arguments[0];
  }
  return O.val()->Construct(e, arguments);
}

// 15.5.1.1 String ( [ value ] )
Handle<JSValue> Call__StringConstructor(
  Error* e, Handle<StringConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0)
    return String::Empty();
  return ::es::ToString(e, arguments[0]);
}

}  // namespace es

#endif