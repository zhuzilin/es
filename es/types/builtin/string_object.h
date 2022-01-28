#ifndef ES_TYPES_BUILTIN_STRING_OBJECT
#define ES_TYPES_BUILTIN_STRING_OBJECT

#include <math.h>

#include <es/types/object.h>
#include <es/parser/character.h>

namespace es {

Handle<String> ToString(Error* e, Handle<JSValue> input);
double ToInteger(Error* e, Handle<JSValue> input);
double ToUint16(Error* e, Handle<JSValue> input);
Handle<String> NumberToString(double m);

class StringProto : public JSObject {
 public:
  static Handle<StringProto> Instance() {
    static Handle<StringProto> singleton = StringProto::New();
    return singleton;
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsObject()) {
      *e = *Error::TypeError(u"String.prototype.toString called with non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
    if (obj.val()->obj_type() != JSObject::OBJ_STRING) {
      *e = *Error::TypeError(u"String.prototype.toString called with non-string");
      return Handle<JSValue>();
    }
    return obj.val()->PrimitiveValue();
  }

  static Handle<JSValue> valueOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsObject()) {
      *e = *Error::TypeError(u"String.prototype.valueOf called with non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
    if (obj.val()->obj_type() != JSObject::OBJ_STRING) {
      *e = *Error::TypeError(u"String.prototype.valueOf called with non-string");
      return Handle<JSValue>();
    }
    return obj.val()->PrimitiveValue();
  }

  static Handle<JSValue> charAt(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return String::Empty();
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    int position = ToInteger(e, vals[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    if (position < 0 || position >= S.val()->size())
      return String::Empty();
    return S.val()->substr(position, 1);
  }

  static Handle<JSValue> charCodeAt(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    int position = ToInteger(e, vals[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    if (position < 0 || position >= S.val()->size())
      return Number::NaN();
    return Number::New((double)((*S.val())[position]));
  }

  static Handle<JSValue> concat(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    std::u16string R = S.val()->data();
    std::vector<Handle<JSValue>> args = vals;
    for (auto arg : args) {
      std::u16string next = ToU16String(e, arg);
      if (!e->IsOk()) return Handle<JSValue>();
      R += next;
    }
    return String::New(R);
  }

  static Handle<JSValue> indexOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    std::u16string S = ToU16String(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<JSValue> search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ToU16String(e, search_string);
    if (!e->IsOk()) return Handle<JSValue>();
    double pos;
    if (vals.size() < 2 || vals[1].val()->IsUndefined())
      pos = 0;
    else {
      pos = ToInteger(e, vals[1]);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    int start = fmin(fmax(pos, 0), S.size());
    size_t find_pos = S.find(search_str, start);
    if (find_pos != std::u16string::npos) {
      return Number::New(find_pos);
    }
    return Number::New(-1);
  }

  static Handle<JSValue> lastIndexOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    std::u16string S = ToU16String(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<JSValue> search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ToU16String(e, search_string);
    if (!e->IsOk()) return Handle<JSValue>();
    double pos;
    if (vals.size() < 2 || vals[1].val()->IsUndefined())
      pos = nan("");
    else {
      pos = ToNumber(e, vals[1]);
      if (!e->IsOk()) return Handle<JSValue>();
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

  static Handle<JSValue> localeCompare(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> match(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> replace(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> search(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> slice(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> split(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> substring(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    int len = S.val()->size();
    if (!e->IsOk()) return Handle<JSValue>();
    int int_start = ToInteger(e, vals[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    int int_end;
    if (vals.size() < 2 || vals[0].val()->IsUndefined()) {
      int_end = S.val()->size();
    } else {
      int_end = ToInteger(e, vals[1]);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    int final_start = fmin(fmax(int_start, 0), len);
    int final_end = fmin(fmax(int_end, 0), len);
    int from = fmin(final_start, final_end);
    int to = fmax(final_start, final_end);
    return S.val()->substr(from, to - from);
  }

  static Handle<JSValue> toLowerCase(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    std::u16string L = S.val()->data();
    std::transform(L.begin(), L.end(), L.begin(), character::ToLowerCase);
    return String::New(L);
  }

  static Handle<JSValue> toLocaleLowerCase(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toLowerCase(e, this_arg, vals);
  }

  static Handle<JSValue> toUpperCase(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    val.val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<String> S = ::es::ToString(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    std::u16string U = S.val()->data();
    std::transform(U.begin(), U.end(), U.begin(), character::ToUpperCase);
    return String::New(U);
  }

  static Handle<JSValue> toLocaleUpperCase(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toUpperCase(e, this_arg, vals);
  }

  static Handle<JSValue> trim(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<StringProto> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"String", true, String::Empty(), false, false, nullptr, 0);
    return Handle<StringProto>(new (jsobj.val()) StringProto());
  }
};

class StringObject : public JSObject {
 public:
  static Handle<StringObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_STRING, u"String", true, primitive_value, false, false, nullptr, 0
    );
    Handle<StringObject> obj = Handle<StringObject>(new (jsobj.val()) StringObject());
    obj.val()->SetPrototype(StringProto::Instance());
    assert(primitive_value.val()->IsString());
    double length = static_cast<Handle<String>>(primitive_value).val()->size();
    obj.val()->AddValueProperty(String::Length(), Number::New(length), false, false, false);
    return obj;
  }

  Handle<JSValue> GetOwnProperty(Handle<String> P) override {
    Handle<JSValue> val = JSObject::GetOwnProperty(P);
    if (!val.val()->IsUndefined())
      return val;
    Error* e = Error::Ok();
    int index = ToInteger(e, P);  // this will never has error.
    if (*NumberToString(fabs(index)).val() != *P.val())
      return Undefined::Instance();
    std::u16string str = static_cast<Handle<String>>(PrimitiveValue()).val()->data();
    int len = str.size();
    if (len <= index)
      return Undefined::Instance();
    Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
    desc.val()->SetDataDescriptor(String::New(str.substr(index, 1)), true, false, false);
    return desc;
  }

};

class StringConstructor : public JSObject {
 public:
  static Handle<StringConstructor> Instance() {
    static Handle<StringConstructor> singleton = StringConstructor::New();
    return singleton;
  }

  // 15.5.1.1 String ( [ value ] )
  Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) override {
    if (arguments.size() == 0)
      return String::Empty();
    return ::es::ToString(e, arguments[0]);
  }

  // 15.5.2.1 String::New ( [ value ] )
  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    if (arguments.size() == 0)
      return StringObject::New(String::Empty());
    Handle<String> str = ::es::ToString(e, arguments[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    return StringObject::New(str);
  }

  static Handle<JSValue> fromCharCode(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    std::u16string result = u"";
    for (Handle<JSValue> val : vals) {
      char16_t c = ToUint16(e, val);
      if (!e->IsOk()) return Handle<JSValue>();
      result += c;
    }
    return String::New(result);
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function String() { [native code] }");
  }

 private:
  static Handle<StringConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"String", true, Handle<JSValue>(), true, true, nullptr, 0);
    return Handle<StringConstructor>(new (jsobj.val()) StringConstructor());
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_STRING_OBJECT