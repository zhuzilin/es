#ifndef ES_TYPES_BUILTIN_ARRAY_OBJECT
#define ES_TYPES_BUILTIN_ARRAY_OBJECT

#include <algorithm>

#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/utils/helper.h>

namespace es {

bool ToBoolean(Handle<JSValue> input);
double ToNumber(Error* e, Handle<JSValue> input);
double ToInteger(Error* e, Handle<JSValue> input);
double ToUint32(Error* e, Handle<JSValue> input);
std::u16string ToU16String(Error* e, Handle<JSValue> input);
std::u16string NumberToU16String(double m);
double StringToNumber(Handle<String> source);
Handle<JSObject> ToObject(Error* e, Handle<JSValue> input);

bool IsArrayIndex(Handle<String> P) {
  return *P.val() == *NumberToString(uint32_t(StringToNumber(P))).val();
}

class ArrayProto : public JSObject {
 public:
  static  Handle<ArrayProto> Instance() {
    static  Handle<ArrayProto> singleton = ArrayProto::New();
    return singleton;
  }

  // 15.4.4.2 Array.prototype.toString ( )
  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSObject> array = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<JSValue> func = array.val()->Get(e, String::New(u"join"));
    if (!e->IsOk()) return Handle<JSValue>();
    if (!func.val()->IsCallable()) {
      func = ObjectProto::Instance().val()->Get(e, String::New(u"toString"));
      if (!e->IsOk()) return Handle<JSValue>();
    }
    return static_cast<Handle<JSObject>>(func).val()->Call(e, this_arg, vals);
  }

  static Handle<JSValue> toLocaleString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> concat(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> join(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSObject> O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<Number> len_val = static_cast<Handle<Number>>(O.val()->Get(e, String::Length()));
    size_t len = len_val.val()->data();

    std::u16string sep = u",";
    if (vals.size() > 0 && !vals[0].val()->IsUndefined()) {
      sep = ToU16String(e, vals[0]);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    if (len == 0)
      return String::Empty();
    Handle<JSValue> element0 = O.val()->Get(e, String::New(u"0"));
    if (!e->IsOk()) return Handle<JSValue>();
    std::u16string R = u"";
    if (!element0.val()->IsUndefined() && !element0.val()->IsNull()) {
      R = ToU16String(e, element0);
    }
    for (double k = 1; k < len; k++) {
      Handle<JSValue> element = O.val()->Get(e, NumberToString(k));
      if (!e->IsOk()) return Handle<JSValue>();
      std::u16string next = u"";
      if (!element.val()->IsUndefined() && !element.val()->IsNull()) {
        next = ToU16String(e, element);
      }
      R += sep + next;
    }
    return String::New(R);
  }

  // 15.4.4.6 Array.prototype.pop ( )
  static Handle<JSValue> pop(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSObject> O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return Handle<JSValue>();
    size_t len = ToNumber(e, O.val()->Get(e, String::Length()));
    if (!e->IsOk()) return Handle<JSValue>();
    if (len == 0) {
      O.val()->Put(e, String::Length(), Number::Zero(), true);
      if (!e->IsOk()) return Handle<JSValue>();
      return Undefined::Instance();
    } else {
      assert(len > 0);
      Handle<String> indx = NumberToString(len - 1);
      Handle<JSValue> element = O.val()->Get(e, indx);
      if (!e->IsOk()) return Handle<JSValue>();
      O.val()->Delete(e, indx, true);
      if (!e->IsOk()) return Handle<JSValue>();
      O.val()->Put(e, String::Length(), Number::New(len - 1), true);
      if (!e->IsOk()) return Handle<JSValue>();
      return element;
    }
  }

  // 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
  static Handle<JSValue> push(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSObject> O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return Handle<JSValue>();
    double n = ToNumber(e, O.val()->Get(e, String::Length()));
    if (!e->IsOk()) return Handle<JSValue>();
    for (Handle<JSValue> E : vals) {
      O.val()->Put(e, NumberToString(n), E, true);
      if (!e->IsOk()) return Handle<JSValue>();
      n++;      
    }
    Handle<Number> num = Number::New(n);
    O.val()->Put(e, String::Length(), num, true);
    if (!e->IsOk()) return Handle<JSValue>();
    return num;
  }

  static Handle<JSValue> reverse(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  // 15.4.4.9 Array.prototype.shift ( )
  static Handle<JSValue> shift(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSObject> O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return Handle<JSValue>();
    size_t len = ToNumber(e, O.val()->Get(e, String::Length()));
    if (!e->IsOk()) return Handle<JSValue>();
    if (len == 0) {
      O.val()->Put(e, String::Length(), Number::Zero(), true);
      if (!e->IsOk()) return Handle<JSValue>();
      return Undefined::Instance();
    }
    Handle<JSValue> first = O.val()->Get(e, String::Zero());
    if (!e->IsOk()) return Handle<JSValue>();
    size_t k = 1;
    while (k < len) {  // 7
      Handle<String> from = NumberToString(k);
      Handle<String> to = NumberToString(k - 1);
      bool from_present = O.val()->HasProperty(from);
      if (from_present) {
        Handle<JSValue> from_val = O.val()->Get(e, from);
        if (!e->IsOk()) return Handle<JSValue>();
        O.val()->Put(e, to, from_val, true);
        if (!e->IsOk()) return Handle<JSValue>();
      } else {
        O.val()->Delete(e, to, true);
        if (!e->IsOk()) return Handle<JSValue>();
      }
      k++;
    }
    O.val()->Delete(e, NumberToString(len - 1), true);
    if (!e->IsOk()) return Handle<JSValue>();
    O.val()->Put(e, String::Length(), Number::New(len - 1), true);
    if (!e->IsOk()) return Handle<JSValue>();
    return first;
  }

  static Handle<JSValue> slice(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> sort(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> splice(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> unshift(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> indexOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> lastIndexOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> every(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> some(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> forEach(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> toLocaleUpperCase(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> map(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> filter(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> reduce(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> reduceRight(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<ArrayProto> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_ARRAY, u"Array", true, Handle<JSValue>(), false, false, nullptr, 0);
    return Handle<ArrayProto>(new (jsobj.val()) ArrayProto());
  }
};

class ArrayObject : public JSObject {
 public:
  static Handle<ArrayObject> New(double len) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_ARRAY, u"Array", true, Handle<JSValue>(), false, false, nullptr, 0
    );
    Handle<ArrayObject> obj(new (jsobj.val()) ArrayObject());
    obj.val()->SetPrototype(ArrayProto::Instance());
    Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
    // Not using AddValueProperty here to by pass the override DefineOwnProperty
    desc.val()->SetDataDescriptor(Number::New(len), true, false, false);
    obj.val()->JSObject::DefineOwnProperty(nullptr, String::Length(), desc, false);
    return obj;
  }

  bool DefineOwnProperty(Error* e, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag) override {
    auto old_len_desc = static_cast<Handle<PropertyDescriptor>>(GetOwnProperty(String::Length()));
    assert(!old_len_desc.val()->IsUndefined());
    double old_len = ToNumber(e, old_len_desc.val()->Value());
    if (*P.val() == *String::Length().val()) {  // 3
      if (!desc.val()->HasValue()) {  // 3.a
        return JSObject::DefineOwnProperty(e, String::Length(), desc, throw_flag);
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
        return JSObject::DefineOwnProperty(e, String::Length(), new_len_desc, throw_flag);
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
      bool succeeded = JSObject::DefineOwnProperty(e, String::Length(), new_len_desc, throw_flag);
      if (!succeeded) return false;  // 3.k
      while (new_len < old_len) {  // 3.l
        old_len--;
        bool delete_succeeded = Delete(e, ::es::ToString(e, Number::New(old_len)), false);
        if (!delete_succeeded) {  // 3.l.iii
          new_len_desc.val()->SetValue(Number::New(old_len + 1));
          if (!new_writable)  // 3.l.iii.2
            new_len_desc.val()->SetWritable(false);
          JSObject::DefineOwnProperty(e, String::Length(), new_len_desc, false);
          goto reject;  // 3.l.iii.4
        }
      }
      if (!new_writable) {  // 3.m
        auto tmp = PropertyDescriptor::New();
        tmp.val()->SetWritable(false);
        assert(JSObject::DefineOwnProperty(e, String::Length(), new_len_desc, false));
        return true;
      }
      return true;  // 3.n
    } else {
      if (IsArrayIndex(P)) {  // 4
        double index = StringToNumber(P);
        if (index >= old_len && !old_len_desc.val()->Writable())  // 4.b
          goto reject;
        bool succeeded = JSObject::DefineOwnProperty(e, P, desc, false);
        if (!succeeded)
          goto reject;
        if (index >= old_len) {  // 4.e
          old_len_desc.val()->SetValue(Number::New(index + 1));
          return JSObject::DefineOwnProperty(e, String::Length(), old_len_desc, false);
        }
        return true;
      }
    }
    return JSObject::DefineOwnProperty(e, P, desc, throw_flag);
reject:
    log::PrintSource("Array::DefineOwnProperty reject " + P.ToString() + " " + desc.ToString());
    if (throw_flag) {
      *e = *Error::TypeError();
    }
    return false;
  }

  std::string ToString() override {
    size_t num = ToNumber(nullptr, Get(nullptr, String::Length()));
    return "Array(" + std::to_string(num) + ")";
  }
};

class ArrayConstructor : public JSObject {
 public:
  static  Handle<ArrayConstructor> Instance() {
    static  Handle<ArrayConstructor> singleton = ArrayConstructor::New();
    return singleton;
  }

  // 15.5.1.1 Array ( [ value ] )
  Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) override {
    return Construct(e, arguments);
  }

  // 15.5.2.1 new Array ( [ value ] )
  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    if (arguments.size() == 1 && arguments[0].val()->IsNumber()) {
      Handle<Number> len = static_cast<Handle<Number>>(arguments[0]);
      if (len.val()->data() == ToUint32(e, len)) {
        return ArrayObject::New(len.val()->data());
      } else {
        *e = *Error::RangeError(u"Invalid array length");
        return Handle<JSValue>();
      }
    }
    Handle<ArrayObject> arr = ArrayObject::New(arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
      Handle<JSValue> arg = arguments[i];
      arr.val()->AddValueProperty(NumberToU16String(i), arg, true, true, true);
    }
    return arr;
  }

  static Handle<JSValue> isArray(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0 || !vals[0].val()->IsObject())
      return Bool::False();
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(vals[0]);
    return Bool::Wrap(obj.val()->Class().val()->data() == u"Array");
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Array() { [native code] }");
  }

 private:
  static Handle<ArrayConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"Array", true, Handle<JSValue>(), true, true, nullptr, 0);
    return Handle<ArrayConstructor>(new (jsobj.val()) ArrayConstructor());
  }
};

// 15.4.4.4 Array.prototype.concat ( [ item1 [ , item2 [ , … ] ] ] )
Handle<JSValue> ArrayProto::concat(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  std::vector<Handle<JSValue>> items = {Runtime::TopValue()};
  items.insert(items.end(), vals.begin(), vals.end());
  Handle<ArrayObject> A = ArrayObject::New(0);
  size_t n = 0;
  for (auto E : items) {
    if (E.val()->IsArrayObject()) {
      Handle<ArrayObject> O = static_cast<Handle<ArrayObject>>(E);
      size_t len = ToNumber(e, O.val()->Get(e, String::Length()));
      if (!e->IsOk()) return Handle<JSValue>();
      for (size_t k = 0; k < len; k++) {  // 5.b.iii
        Handle<String> P = NumberToString(k);
        if (O.val()->HasProperty(P)) {
          Handle<JSValue> sub_element = O.val()->Get(e, P);
          if (!e->IsOk()) return Handle<JSValue>();
          A.val()->AddValueProperty(NumberToU16String(n), sub_element, true, true, true);
        }
        n++;
      }
    } else {
      A.val()->AddValueProperty(NumberToU16String(n), E, true, true, true);
      n++;
    }
  }
  return A;
}

// 15.4.4.10 Array.prototype.slice (start, end)
Handle<JSValue> ArrayProto::slice(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSObject> O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return Handle<JSValue>();
  double len = ToNumber(e, O.val()->Get(e, String::Length()));
  Handle<ArrayObject> A = ArrayObject::New(0);
  if (vals.size() == 0 || vals[0].val()->IsUndefined()) {
    *e = *Error::TypeError(u"start of Array.prototype.slice cannot be undefined");
  }
  int relative_start = ToInteger(e, vals[0]);
  if (!e->IsOk()) return Handle<JSValue>();
  int k;
  if (relative_start < 0)
    k = fmax(relative_start + len, 0);
  else
    k = fmin(relative_start, len);
  int relative_end;
  if (vals.size() < 2 || vals[1].val()->IsUndefined()) {
    relative_end = len;
  } else {
    relative_end = ToInteger(e, vals[1]);
    if (!e->IsOk()) return Handle<JSValue>();
  }
  int final;
  int n = 0;
  if (relative_end < 0)
    final = fmax(relative_end + len, 0);
  else
    final = fmin(relative_end, len);
  while (k < final) {
    Handle<String> Pk = NumberToString(k);
    if (O.val()->HasProperty(Pk)) {
      Handle<JSValue> k_value = O.val()->Get(e, Pk);
      if (!e->IsOk()) return Handle<JSValue>();
      A.val()->AddValueProperty(NumberToU16String(n), k_value, true, true, true);
    }
    k++;
    n++;
  }
  return A;
}

// 15.4.4.11 Array.prototype.sort (comparefn)
Handle<JSValue> ArrayProto::sort(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSObject> obj = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return Handle<JSValue>();
  size_t len = ToNumber(e, obj.val()->Get(e, String::Length()));
  // TODO(zhuzilin) Check the implementation dependecy cases would not cause error.
  Handle<JSValue> comparefn;
  if (vals.size() == 0 || vals[0].val()->IsUndefined()) {
    comparefn = Undefined::Instance();
  } else if (vals[0].val()->IsCallable()) {
    comparefn = static_cast<Handle<JSObject>>(vals[0]);
  } else {
    *e = *Error::TypeError(u"comparefn of Array.prototype.sort is not callable");
    return Handle<JSValue>();
  }
  std::vector<std::pair<bool, Handle<JSValue>>> indices;
  for (size_t i = 0; i < len; i++) {
    Handle<String> istr = NumberToString(i);
    if (obj.val()->HasProperty(istr)) {
      Handle<JSValue> val = obj.val()->Get(e, istr);
      if (!e->IsOk()) return Handle<JSValue>();
      indices.emplace_back(std::make_pair(true, val));
    } else {
      indices.emplace_back(std::make_pair(false, Handle<JSValue>()));
    }
  }
  std::sort(indices.begin(), indices.end(),
    // SortCompare
    [&] (const auto& pair_x, const auto& pair_y) {
      bool hasj = pair_x.first;
      bool hask = pair_y.first;
      if (!hask)
        return false;
      if (!hasj && hask)
        return true;
      Handle<JSValue> x = pair_x.second;
      Handle<JSValue> y = pair_y.second;
      if (y.val()->IsUndefined())
        return false;
      if (x.val()->IsUndefined() && !y.val()->IsUndefined())
        return true;
      if (!comparefn.val()->IsUndefined()) {
        Handle<JSValue> res = static_cast<Handle<JSObject>>(comparefn).val()->Call(e, Undefined::Instance(), {x, y});
        if (!e->IsOk()) return false;
        return ToNumber(e, res) < 0;
      }
      // NOTE(zhuzilin) 123 will be smaller 21... which I've checked with V8.
      std::u16string xstr = ToU16String(e, x);
      std::u16string ystr = ToU16String(e, y);
      return xstr < ystr;
    });
  if (!e->IsOk()) return Handle<JSValue>();
  for (size_t i = 0; i < len; i++) {
    Handle<String> istr = NumberToString(i);
    bool has = indices[i].first;
    Handle<JSValue> val = indices[i].second;
    if (indices[i].first) {
      obj.val()->Put(e, istr, val, true);
      if (!e->IsOk()) return Handle<JSValue>();
    } else {
      obj.val()->Delete(e, istr, true);
      if (!e->IsOk()) return Handle<JSValue>();
    }
  }
  return obj;
}

// 15.4.4.18 Array.prototype.forEach ( callbackfn [ , thisArg ] )
Handle<JSValue> ArrayProto::forEach(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSObject> O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return Handle<JSValue>();
  size_t len = ToNumber(e, O.val()->Get(e, String::Length()));
  if (vals.size() == 0 || !vals[0].val()->IsCallable()) {  // 4
    *e = *Error::TypeError(u"Array.prototype.forEach called on non-callable");
    return Handle<JSValue>();
  }
  Handle<JSObject> callbackfn = static_cast<Handle<JSObject>>(vals[0]);
  Handle<JSValue> T;
  if (vals.size() < 2) {
    T = Undefined::Instance();
  } else {
    T = vals[1];
  }
  Handle<ArrayObject> A = ArrayObject::New(len);
  for (size_t k = 0; k < len; k++) {
    Handle<String> p_k = NumberToString(k);
    bool k_present = O.val()->HasProperty(p_k);
    if (!e->IsOk()) return Handle<JSValue>();
    if (k_present) {
      Handle<JSValue> k_value = O.val()->Get(e, p_k);
      if (!e->IsOk()) return Handle<JSValue>();
      Handle<JSValue> mapped_value = callbackfn.val()->Call(e, T, {k_value, Number::New(k), O});
      if (!e->IsOk()) return Handle<JSValue>();
    }
  }
  return Undefined::Instance();
}

// 15.4.4.19 Array.prototype.map ( callbackfn [ , thisArg ] )
Handle<JSValue> ArrayProto::map(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSObject> O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return Handle<JSValue>();
  size_t len = ToNumber(e, O.val()->Get(e, String::Length()));
  if (vals.size() == 0 || !vals[0].val()->IsCallable()) {  // 4
    *e = *Error::TypeError(u"Array.prototype.map called on non-callable");
    return Handle<JSValue>();
  }
  Handle<JSObject> callbackfn = static_cast<Handle<JSObject>>(vals[0]);
  Handle<JSValue> T;
  if (vals.size() < 2) {
    T = Undefined::Instance();
  } else {
    T = vals[1];
  }
  Handle<ArrayObject> A = ArrayObject::New(len);
  for (size_t k = 0; k < len; k++) {
    Handle<String> p_k = NumberToString(k);
    bool k_present = O.val()->HasProperty(p_k);
    if (!e->IsOk()) return Handle<JSValue>();
    if (k_present) {
      Handle<JSValue> k_value = O.val()->Get(e, p_k);
      if (!e->IsOk()) return Handle<JSValue>();
      Handle<JSValue> mapped_value = callbackfn.val()->Call(e, T, {k_value, Number::New(k), O});
      if (!e->IsOk()) return Handle<JSValue>();
      A.val()->AddValueProperty(p_k.val()->data(), mapped_value, true, true, true);
    }
  }
  return A;
}

// 15.4.4.20 Array.prototype.filter ( callbackfn [ , thisArg ] )
Handle<JSValue> ArrayProto::filter(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSObject> O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return Handle<JSValue>();
  size_t len = ToNumber(e, O.val()->Get(e, String::Length()));
  if (vals.size() == 0 || !vals[0].val()->IsCallable()) {  // 4
    *e = *Error::TypeError(u"Array.prototype.filter called on non-callable");
    return Handle<JSValue>();
  }
  Handle<JSObject> callbackfn = static_cast<Handle<JSObject>>(vals[0]);
  Handle<JSValue> T;
  if (vals.size() < 2) {
    T = Undefined::Instance();
  } else {
    T = vals[1];
  }
  size_t to = 0;
  Handle<ArrayObject> A = ArrayObject::New(len);
  for (size_t k = 0; k < len; k++) {
    Handle<String> p_k = NumberToString(k);
    bool k_present = O.val()->HasProperty(p_k);
    if (!e->IsOk()) return Handle<JSValue>();
    if (k_present) {
      Handle<JSValue> k_value = O.val()->Get(e, p_k);
      if (!e->IsOk()) return Handle<JSValue>();
      Handle<JSValue> selected = callbackfn.val()->Call(e, T, {k_value, Number::New(k), O});
      if (!e->IsOk()) return Handle<JSValue>();
      if (ToBoolean(selected)) {
        A.val()->AddValueProperty(NumberToU16String(to), k_value, true, true, true);
        to++;
      }
    }
  }
  return A;
}

Handle<JSValue> ObjectConstructor::keys(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  if (vals.size() < 1 || !vals[0].val()->IsObject()) {
    *e = *Error::TypeError(u"Object.keys called on non-object");
    return Handle<JSValue>();
  }
  Handle<JSObject> O = static_cast<Handle<JSObject>>(vals[0]);
  auto properties = O.val()->AllEnumerableProperties();
  size_t n = properties.size();
  Handle<ArrayObject> arr_obj = ArrayObject::New(n);
  for (size_t index = 0; index < n; index++) {
    arr_obj.val()->AddValueProperty(
      NumberToString(index), properties[index].first, true, true, true);
  }
  return arr_obj;
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARRAY_OBJECT