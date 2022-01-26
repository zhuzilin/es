#ifndef ES_TYPES_BUILTIN_STRING_OBJECT
#define ES_TYPES_BUILTIN_STRING_OBJECT

#include <math.h>

#include <es/types/object.h>
#include <es/parser/character.h>

namespace es {

String* ToString(Error* e, JSValue* input);
double ToInteger(Error* e, JSValue* input);
double ToUint16(Error* e, JSValue* input);
String* NumberToString(double m);

class StringProto : public JSObject {
 public:
  static StringProto* Instance() {
    static StringProto* singleton = StringProto::New();
    return singleton;
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (!val->IsObject()) {
      *e = *Error::TypeError(u"String.prototype.toString called with non-object");
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(val);
    if (obj->obj_type() != JSObject::OBJ_STRING) {
      *e = *Error::TypeError(u"String.prototype.toString called with non-string");
      return nullptr;
    }
    return obj->PrimitiveValue();
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (!val->IsObject()) {
      *e = *Error::TypeError(u"String.prototype.valueOf called with non-object");
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(val);
    if (obj->obj_type() != JSObject::OBJ_STRING) {
      *e = *Error::TypeError(u"String.prototype.valueOf called with non-string");
      return nullptr;
    }
    return obj->PrimitiveValue();
  }

  static JSValue* charAt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return String::Empty();
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    String* S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    int position = ToInteger(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    if (position < 0 || position >= S->size())
      return String::Empty();
    return S->substr(position, 1);
  }

  static JSValue* charCodeAt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    String* S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    int position = ToInteger(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    if (position < 0 || position >= S->size())
      return Number::NaN();
    return Number::New((double)((*S)[position]));
  }

  static JSValue* concat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    String* S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    std::u16string R = S->data();
    std::vector<JSValue*> args = vals;
    for (auto arg : args) {
      std::u16string next = ToU16String(e, arg);
      if (!e->IsOk()) return nullptr;
      R += next;
    }
    return String::New(R);
  }

  static JSValue* indexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ToU16String(e, val);
    if (!e->IsOk()) return nullptr;
    JSValue* search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ToU16String(e, search_string);
    if (!e->IsOk()) return nullptr;
    double pos;
    if (vals.size() < 2 || vals[1]->IsUndefined())
      pos = 0;
    else {
      pos = ToInteger(e, vals[1]);
      if (!e->IsOk()) return nullptr;
    }
    int start = fmin(fmax(pos, 0), S.size());
    size_t find_pos = S.find(search_str, start);
    if (find_pos != std::u16string::npos) {
      return Number::New(find_pos);
    }
    return Number::New(-1);
  }

  static JSValue* lastIndexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ToU16String(e, val);
    if (!e->IsOk()) return nullptr;
    JSValue* search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ToU16String(e, search_string);
    if (!e->IsOk()) return nullptr;
    double pos;
    if (vals.size() < 2 || vals[1]->IsUndefined())
      pos = nan("");
    else {
      pos = ToNumber(e, vals[1]);
      if (!e->IsOk()) return nullptr;
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

  static JSValue* localeCompare(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* match(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* replace(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* search(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* slice(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* split(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* substring(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    String* S = ::es::ToString(e, val);
    int len = S->size();
    if (!e->IsOk()) return nullptr;
    int int_start = ToInteger(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    int int_end;
    if (vals.size() < 2 || vals[0]->IsUndefined()) {
      int_end = S->size();
    } else {
      int_end = ToInteger(e, vals[1]);
      if (!e->IsOk()) return nullptr;
    }
    int final_start = fmin(fmax(int_start, 0), len);
    int final_end = fmin(fmax(int_end, 0), len);
    int from = fmin(final_start, final_end);
    int to = fmax(final_start, final_end);
    return S->substr(from, to - from);
  }

  static JSValue* toLowerCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    String* S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    std::u16string L = S->data();
    std::transform(L.begin(), L.end(), L.begin(), character::ToLowerCase);
    return String::New(L);
  }

  static JSValue* toLocaleLowerCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toLowerCase(e, this_arg, vals);
  }

  static JSValue* toUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    String* S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    std::u16string U = S->data();
    std::transform(U.begin(), U.end(), U.begin(), character::ToUpperCase);
    return String::New(U);
  }

  static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toUpperCase(e, this_arg, vals);
  }

  static JSValue* trim(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  static StringProto* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"String", true, String::Empty(), false, false, nullptr, 0);
    return new (jsobj) StringProto();
  }
};

class StringObject : public JSObject {
 public:
  static StringObject* New(JSValue* primitive_value) {
    JSObject* jsobj = JSObject::New(
      OBJ_STRING, u"String", true, primitive_value, false, false, nullptr, 0
    );
    StringObject* obj = new (jsobj) StringObject();
    obj->SetPrototype(StringProto::Instance());
    assert(primitive_value->IsString());
    double length = static_cast<String*>(primitive_value)->data().size();
    obj->AddValueProperty(String::Length(), Number::New(length), false, false, false);
    return obj;
  }

  JSValue* GetOwnProperty(String* P) override {
    JSValue* val = JSObject::GetOwnProperty(P);
    if (!val->IsUndefined())
      return val;
    Error* e = Error::Ok();
    int index = ToInteger(e, P);  // this will never has error.
    if (*NumberToString(fabs(index)) != *P)
      return Undefined::Instance();
    std::u16string str = static_cast<String*>(PrimitiveValue())->data();
    int len = str.size();
    if (len <= index)
      return Undefined::Instance();
    PropertyDescriptor* desc = PropertyDescriptor::New();
    desc->SetDataDescriptor(String::New(str.substr(index, 1)), true, false, false);
    return desc;
  }

};

class StringConstructor : public JSObject {
 public:
  static StringConstructor* Instance() {
    static StringConstructor* singleton = StringConstructor::New();
    return singleton;
  }

  // 15.5.1.1 String ( [ value ] )
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    if (arguments.size() == 0)
      return String::Empty();
    return ::es::ToString(e, arguments[0]);
  }

  // 15.5.2.1 String::New ( [ value ] )
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() == 0)
      return StringObject::New(String::Empty());
    String* str = ::es::ToString(e, arguments[0]);
    if (!e->IsOk()) return nullptr;
    return StringObject::New(str);
  }

  static JSValue* fromCharCode(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    std::u16string result = u"";
    for (JSValue* val : vals) {
      char16_t c = ToUint16(e, val);
      if (!e->IsOk()) return nullptr;
      result += c;
    }
    return String::New(result);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"function String() { [native code] }");
  }

 private:
  static StringConstructor* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"String", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) StringConstructor();
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_STRING_OBJECT