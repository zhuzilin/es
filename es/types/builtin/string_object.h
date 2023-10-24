#ifndef ES_TYPES_BUILTIN_STRING_OBJECT
#define ES_TYPES_BUILTIN_STRING_OBJECT

#include <math.h>

#include <es/types/object.h>
#include <es/parser/character.h>

namespace es {

JSValue ToString(JSValue& e, JSValue input);
double ToInteger(JSValue& e, JSValue input);
double ToUint16(JSValue& e, JSValue input);
JSValue NumberToString(double m);

namespace string_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"String", true, string::Empty(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = string_proto::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (val.IsString()) {
    return val;
  }
  if (!val.IsStringObject()) {
    e = error::TypeError(u"String.prototype.toString called with non-string");
    return JSValue();
  }
  return js_object::PrimitiveValue(val);
}

inline JSValue valueOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (!val.IsObject()) {
    e = error::TypeError(u"String.prototype.valueOf called with non-object");
    return JSValue();
  }
  if (!val.IsStringObject()) {
    e = error::TypeError(u"String.prototype.valueOf called with non-string");
    return JSValue();
  }
  return js_object::PrimitiveValue(val);
}

inline JSValue charAt(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return string::Empty();
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue S = ::es::ToString(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int position = ToInteger(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (position < 0 || (size_t)position >= string::size(S))
    return string::Empty();
  return string::Substr(S, position, 1);
}

inline JSValue charCodeAt(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue S = ::es::ToString(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int position = ToInteger(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (position < 0 || (size_t)position >= string::size(S))
    return number::NaN();
  return number::New((double)(string::data_view(S)[position]));
}

inline JSValue concat(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue S = ::es::ToString(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string R = string::data(S);
  std::vector<JSValue> args = vals;
  for (auto arg : args) {
    std::u16string next = ToU16String(e, arg);
    if (unlikely(!error::IsOk(e))) return JSValue();
    R += next;
  }
  return string::New(R);
}

inline JSValue indexOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string S = ToU16String(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue search_string;
  if (vals.size() == 0)
    search_string = undefined::New();
  else
    search_string = vals[0];
  std::u16string search_str = ToU16String(e, search_string);
  if (unlikely(!error::IsOk(e))) return JSValue();
  double pos;
  if (vals.size() < 2 || vals[1].IsUndefined())
    pos = 0;
  else {
    pos = ToInteger(e, vals[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  int start = fmin(fmax(pos, 0), S.size());
  size_t find_pos = S.find(search_str, start);
  if (find_pos != std::u16string::npos) {
    return number::New(find_pos);
  }
  return number::New(-1);
}

inline JSValue lastIndexOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string S = ToU16String(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue search_string;
  if (vals.size() == 0)
    search_string = undefined::New();
  else
    search_string = vals[0];
  std::u16string search_str = ToU16String(e, search_string);
  if (unlikely(!error::IsOk(e))) return JSValue();
  double pos;
  if (vals.size() < 2 || vals[1].IsUndefined())
    pos = nan("");
  else {
    pos = ToNumber(e, vals[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  int start;
  if (isnan(pos))
    start = S.size();
  else
    start = fmin(fmax(pos, 0), S.size());
  size_t find_pos = S.rfind(search_str, start);
  if (find_pos != std::u16string::npos) {
    return number::New(find_pos);
  }
  return number::New(-1);
}

inline JSValue localeCompare(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue match(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue replace(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue search(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue slice(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue split(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue substring(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue S = ::es::ToString(e, val);
  int len = string::size(S);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int int_start = ToInteger(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int int_end;
  if (vals.size() < 2 || vals[0].IsUndefined()) {
    int_end = string::size(S);
  } else {
    int_end = ToInteger(e, vals[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  int final_start = fmin(fmax(int_start, 0), len);
  int final_end = fmin(fmax(int_end, 0), len);
  int from = fmin(final_start, final_end);
  int to = fmax(final_start, final_end);
  return string::Substr(S, from, to - from);
}

inline JSValue toLowerCase(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue S = ::es::ToString(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string L = string::data(S);
  std::transform(L.begin(), L.end(), L.begin(), character::ToLowerCase);
  return string::New(L);
}

inline JSValue toLocaleLowerCase(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  // TODO(zhuzilin) may need to fix this.
  return toLowerCase(e, this_arg, vals);
}

inline JSValue toUpperCase(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue S = ::es::ToString(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string U = string::data(S);
  std::transform(U.begin(), U.end(), U.begin(), character::ToUpperCase);
  return string::New(U);
}

inline JSValue toLocaleUpperCase(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  // TODO(zhuzilin) may need to fix this.
  return toUpperCase(e, this_arg, vals);
}

inline JSValue trim(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace string_proto

namespace string_object {

inline JSValue New(JSValue primitive_value) {
  JSValue jsobj = js_object::New(
    u"String", true, primitive_value, false, false, nullptr, 0
  );

  jsobj.SetType(OBJ_STRING);
  js_object::SetPrototype(jsobj, string_proto::Instance());
  ASSERT(primitive_value.IsString());
  double length = string::size(primitive_value);
  AddValueProperty(jsobj, string::Length(), number::New(length), false, false, false);
  return jsobj;
}

}  // namespace string_object

namespace string_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"String", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_STRING_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = string_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue fromCharCode(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  std::u16string result = u"";
  for (JSValue val : vals) {
    char16_t c = ToUint16(e, val);
    if (unlikely(!error::IsOk(e))) return JSValue();
    result += c;
  }
  return string::New(result);
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function String() { [native code] }");
}

}  // namespace string_constructor

JSValue GetOwnProperty__String(JSValue O, JSValue P);
JSValue Call__StringConstructor(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Construct__StringConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_STRING_OBJECT