#ifndef ES_TYPES_BUILTIN_STRING_OBJECT
#define ES_TYPES_BUILTIN_STRING_OBJECT

#include <math.h>

#include <es/types/object.h>
#include <es/parser/character.h>

namespace es {

Handle<String> ToString(Handle<Error>& e, Handle<JSValue> input);
double ToInteger(Handle<Error>& e, Handle<JSValue> input);
double ToUint16(Handle<Error>& e, Handle<JSValue> input);
Handle<String> NumberToString(double m);

class StringProto : public JSObject {
 public:
  static Handle<StringProto> Instance() {
    static Handle<StringProto> singleton = StringProto::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsString()) {
      return val;
    }
    if (!val.val()->IsStringObject()) {
      e = Error::TypeError(u"String.prototype.toString called with non-string");
      return Handle<JSValue>();
    }
    return static_cast<Handle<JSObject>>(val).val()->PrimitiveValue();
  }

  static Handle<JSValue> valueOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (unlikely(!val.val()->IsStringObject())) {
      if (!val.val()->IsObject()) {
        e = Error::TypeError(u"String.prototype.valueOf called with non-object");
        return Handle<JSValue>();
      }
      e = Error::TypeError(u"String.prototype.valueOf called with non-string");
      return Handle<JSValue>();
    }
    return static_cast<JSObject*>(val.val())->PrimitiveValue();
  }

  static Handle<JSValue> charAt(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return String::Empty();
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    int position = ToInteger(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (position < 0 || (size_t)position >= S.val()->size())
      return String::Empty();
    return String::Substr(S, position, 1);
  }

  static Handle<JSValue> charCodeAt(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    int position = ToInteger(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (position < 0 || (size_t)position >= S.val()->size())
      return Number::NaN();
    return Number::New((double)(S.val()->get(position)));
  }

  static Handle<JSValue> concat(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    std::vector<Handle<JSValue>> args = vals;
    std::vector<Handle<String>> strs = {S};
    for (auto arg : args) {
      strs.emplace_back(::es::ToString(e, arg));
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    return String::Concat(strs);
  }

  static Handle<JSValue> indexOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    std::u16string S = ToU16String(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<JSValue> search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ToU16String(e, search_string);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    double pos;
    if (vals.size() < 2 || vals[1].val()->IsUndefined())
      pos = 0;
    else {
      pos = ToInteger(e, vals[1]);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    int start = fmin(fmax(pos, 0), S.size());
    size_t find_pos = S.find(search_str, start);
    if (find_pos != std::u16string::npos) {
      return Number::New(find_pos);
    }
    return Number::New(-1);
  }

  static Handle<JSValue> lastIndexOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    std::u16string S = ToU16String(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<JSValue> search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ToU16String(e, search_string);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    double pos;
    if (vals.size() < 2 || vals[1].val()->IsUndefined())
      pos = nan("");
    else {
      pos = ToNumber(e, vals[1]);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    int start;
    if (isnan(pos))
      start = S.size();
    else
      start = fmin(fmax(pos, 0), S.size());
    size_t find_pos = S.rfind(search_str, start);
    if (find_pos != std::u16string::npos) {
      return Number::New(find_pos);
    }
    return Number::New(-1);
  }

  static Handle<JSValue> localeCompare(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> match(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> replace(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> search(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> slice(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> split(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> substring(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    int len = S.val()->size();
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    int int_start = ToInteger(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    int int_end;
    if (vals.size() < 2 || vals[0].val()->IsUndefined()) {
      int_end = S.val()->size();
    } else {
      int_end = ToInteger(e, vals[1]);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    int final_start = fmin(fmax(int_start, 0), len);
    int final_end = fmin(fmax(int_end, 0), len);
    int from = fmin(final_start, final_end);
    int to = fmax(final_start, final_end);
    return String::Substr(S, from, to - from);
  }

  static Handle<JSValue> toLowerCase(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    std::u16string L = S.val()->data();
    std::transform(L.begin(), L.end(), L.begin(), character::ToLowerCase);
    return String::New(L);
  }

  static Handle<JSValue> toLocaleLowerCase(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toLowerCase(e, this_arg, vals);
  }

  static Handle<JSValue> toUpperCase(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    CheckObjectCoercible(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    std::u16string U = S.val()->data();
    std::transform(U.begin(), U.end(), U.begin(), character::ToUpperCase);
    return String::New(U);
  }

  static Handle<JSValue> toLocaleUpperCase(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toUpperCase(e, this_arg, vals);
  }

  static Handle<JSValue> trim(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  template<flag_t flag>
  static Handle<StringProto> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      u"String", true, String::Empty(), false, false, nullptr);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<StringProto>(jsobj);
  }
};

class StringObject : public JSObject {
 public:
  static Handle<StringObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New<0>(
      u"String", true, primitive_value, false, false, nullptr
    );

    jsobj.val()->SetType(OBJ_STRING);
    Handle<StringObject> obj = Handle<StringObject>(jsobj);
    obj.val()->SetPrototype(StringProto::Instance());
    ASSERT(primitive_value.val()->IsString());
    double length = static_cast<Handle<String>>(primitive_value).val()->size();
    AddValueProperty(obj, String::Length(), Number::New(length), false, false, false);
    return obj;
  }
};

class StringConstructor : public JSObject {
 public:
  static Handle<StringConstructor> Instance() {
    static Handle<StringConstructor> singleton = StringConstructor::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> fromCharCode(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    std::u16string result = u"";
    for (Handle<JSValue> val : vals) {
      char16_t c = ToUint16(e, val);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      result += c;
    }
    return String::New(result);
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function String() { [native code] }");
  }

 private:
  template<flag_t flag>
  static Handle<StringConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      u"String", true, Handle<JSValue>(), true, true, nullptr);

    jsobj.val()->SetType(OBJ_STRING_CONSTRUCTOR);
    return Handle<StringConstructor>(jsobj);
  }
};

StackPropertyDescriptor GetOwnProperty__String(Handle<StringObject> O, Handle<String> P);
Handle<JSValue> Call__StringConstructor(Handle<Error>& e, Handle<StringConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__StringConstructor(Handle<Error>& e, Handle<StringConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_STRING_OBJECT