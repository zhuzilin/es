#ifndef ES_TYPES_BUILTIN_ARRAY_OBJECT
#define ES_TYPES_BUILTIN_ARRAY_OBJECT

#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/utils/helper.h>

namespace es {

bool ToBoolean(JSValue* input);
double ToNumber(Error* e, JSValue* input);
double ToUint32(Error* e, JSValue* input);
std::u16string ToString(Error* e, JSValue* input);
std::u16string NumberToString(double m);
double StringToNumber(std::u16string source);
JSObject* ToObject(Error* e, JSValue* input);

bool IsArrayIndex(std::u16string P) {
  return P == NumberToString(uint32_t(StringToNumber(P)));
}

class ArrayProto : public JSObject {
 public:
  static  ArrayProto* Instance() {
    static  ArrayProto singleton;
    return &singleton;
  }

  // 15.4.4.2 Array.prototype.toString ( )
  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSObject* array = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return nullptr;
    JSValue* func = array->Get(e, u"join");
    if (!e->IsOk()) return nullptr;
    if (!func->IsCallable()) {
      func = ObjectProto::Instance()->Get(e, u"toString");
      if (!e->IsOk()) return nullptr;
    }
    return static_cast<JSObject*>(func)->Call(e, this_arg, vals);
  }

  static JSValue* toLocaleString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* concat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* join(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSObject* O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return nullptr;
    Number* len_val = static_cast<Number*>(O->Get(e, u"length"));
    size_t len = len_val->data();

    std::u16string sep = u",";
    if (vals.size() > 0 && !vals[0]->IsUndefined()) {
      sep = ::es::ToString(e, vals[0]);
      if (!e->IsOk()) return nullptr;
    }
    if (len == 0)
      return String::Empty();
    JSValue* element0 = O->Get(e, u"0");
    if (!e->IsOk()) return nullptr;
    std::u16string R = u"";
    if (!element0->IsUndefined() && !element0->IsNull()) {
      R = ::es::ToString(e, element0);
    }
    for (double k = 1; k < len; k++) {
      JSValue* element = O->Get(e, NumberToString(k));
      if (!e->IsOk()) return nullptr;
      std::u16string next = u"";
      if (!element->IsUndefined() && !element->IsNull()) {
        next = ::es::ToString(e, element);
      }
      R += sep + next;
    }
    return new String(R);
  }

  // 15.4.4.6 Array.prototype.pop ( )
  static JSValue* pop(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSObject* O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return nullptr;
    size_t len = ToNumber(e, O->Get(e, u"length"));
    if (!e->IsOk()) return nullptr;
    if (len == 0) {
      O->Put(e, u"length", Number::Zero(), true);
      if (!e->IsOk()) return nullptr;
      return Undefined::Instance();
    } else {
      assert(len > 0);
      std::u16string indx = NumberToString(len - 1);
      JSValue* element = O->Get(e, indx);
      if (!e->IsOk()) return nullptr;
      O->Delete(e, indx, true);
      if (!e->IsOk()) return nullptr;
      O->Put(e, u"length", new Number(len - 1), true);
      if (!e->IsOk()) return nullptr;
      return element;
    }
  }

  // 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
  static JSValue* push(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSObject* O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return nullptr;
    double n = ToNumber(e, O->Get(e, u"length"));
    if (!e->IsOk()) return nullptr;
    for (JSValue* E : vals) {
      O->Put(e, NumberToString(n), E, true);
      if (!e->IsOk()) return nullptr;
      n++;      
    }
    Number* num = new Number(n);
    O->Put(e, u"length", num, true);
    if (!e->IsOk()) return nullptr;
    return num;
  }

  static JSValue* reverse(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  // 15.4.4.9 Array.prototype.shift ( )
  static JSValue* shift(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSObject* O = ToObject(e, Runtime::TopValue());
    if (!e->IsOk()) return nullptr;
    size_t len = ToNumber(e, O->Get(e, u"length"));
    if (!e->IsOk()) return nullptr;
    if (len == 0) {
      O->Put(e, u"length", Number::Zero(), true);
      if (!e->IsOk()) return nullptr;
      return Undefined::Instance();
    }
    JSValue* first = O->Get(e, u"0");
    if (!e->IsOk()) return nullptr;
    size_t k = 1;
    while (k < len) {  // 7
      std::u16string from = NumberToString(k);
      std::u16string to = NumberToString(k - 1);
      bool from_present = O->HasProperty(from);
      if (from_present) {
        JSValue* from_val = O->Get(e, from);
        if (!e->IsOk()) return nullptr;
        O->Put(e, to, from_val, true);
        if (!e->IsOk()) return nullptr;
      } else {
        O->Delete(e, to, true);
        if (!e->IsOk()) return nullptr;
      }
      k++;
    }
    O->Delete(e, NumberToString(len - 1), true);
    if (!e->IsOk()) return nullptr;
    O->Put(e, u"length", new Number(len - 1), true);
    if (!e->IsOk()) return nullptr;
    return first;
  }

  static JSValue* slice(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* sort(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* splice(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* unshift(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* indexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* lastIndexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* every(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* some(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* forEach(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* map(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* filter(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* reduce(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* reduceRight(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   ArrayProto() :
    JSObject(
      OBJ_ARRAY, u"Array", true, nullptr, false, false
    ) {

    }
};

class ArrayObject : public JSObject {
 public:
  ArrayObject(double len) :
    JSObject(OBJ_ARRAY, u"Array", true, nullptr, false, false) {
    SetPrototype(ArrayProto::Instance());
    // Not using AddValueProperty here to by pass the override DefineOwnProperty
    PropertyDescriptor* desc = new PropertyDescriptor();
    desc->SetDataDescriptor(new Number(len), true, false, false);
    JSObject::DefineOwnProperty(nullptr, u"length", desc, false);
  }

  bool DefineOwnProperty(Error* e, std::u16string P, PropertyDescriptor* desc, bool throw_flag) override {
    auto old_len_desc = static_cast<PropertyDescriptor*>(GetOwnProperty(u"length"));
    assert(!old_len_desc->IsUndefined());
    double old_len = ToNumber(e, old_len_desc->Value());
    if (P == u"length") {  // 3
      if (!desc->HasValue()) {  // 3.a
        return JSObject::DefineOwnProperty(e, u"length", desc, throw_flag);
      }
      PropertyDescriptor* new_len_desc = new PropertyDescriptor();
      new_len_desc->Set(desc);
      double new_len = ToUint32(e, desc->Value());
      if (!e->IsOk()) goto reject;
      double new_num = ToNumber(e, desc->Value());
      if (!e->IsOk()) goto reject;
      if (new_len != new_num) {
        *e = *Error::RangeError(u"length of array need to be uint32.");
        return false;
      }
      new_len_desc->SetValue(new Number(new_len));
      if (new_len >= old_len) {  // 3.f
        return JSObject::DefineOwnProperty(e, u"length", new_len_desc, throw_flag);
      }
      if (!old_len_desc->Writable())  // 3.g
        goto reject;
      bool new_writable;
      if (new_len_desc->HasWritable() && new_len_desc->Writable()) {  // 3.h
        new_writable = true;
      } else {  // 3.l
        new_writable = false;
        new_len_desc->SetWritable(true);
      }
      bool succeeded = JSObject::DefineOwnProperty(e, u"length", new_len_desc, throw_flag);
      if (!succeeded) return false;  // 3.k
      while (new_len < old_len) {  // 3.l
        old_len--;
        bool delete_succeeded = Delete(e, ::es::ToString(e, new Number(old_len)), false);
        if (!delete_succeeded) {  // 3.l.iii
          new_len_desc->SetValue(new Number(old_len + 1));
          if (!new_writable)  // 3.l.iii.2
            new_len_desc->SetWritable(false);
          JSObject::DefineOwnProperty(e, u"length", new_len_desc, false);
          goto reject;  // 3.l.iii.4
        }
      }
      if (!new_writable) {  // 3.m
        auto tmp = new PropertyDescriptor();
        tmp->SetWritable(false);
        assert(JSObject::DefineOwnProperty(e, u"length", new_len_desc, false));
        return true;
      }
      return true;  // 3.n
    } else {
      if (IsArrayIndex(P)) {  // 4
        double index = StringToNumber(P);
        if (index >= old_len && !old_len_desc->Writable())  // 4.b
          goto reject;
        bool succeeded = JSObject::DefineOwnProperty(e, P, desc, false);
        if (!succeeded)
          goto reject;
        if (index >= old_len) {  // 4.e
          old_len_desc->SetValue(new Number(index + 1));
          return JSObject::DefineOwnProperty(e, u"length", old_len_desc, false);
        }
        return true;
      }
    }
    return JSObject::DefineOwnProperty(e, P, desc, throw_flag);
reject:
    log::PrintSource("Array::DefineOwnProperty reject ", P, " " + desc->ToString());
    if (throw_flag) {
      *e = *Error::TypeError();
    }
    return false;
  }

  std::string ToString() override {
    size_t num = ToNumber(nullptr, Get(nullptr, u"length"));
    return "Array(" + std::to_string(num) + ")";
  }
};

class ArrayConstructor : public JSObject {
 public:
  static  ArrayConstructor* Instance() {
    static  ArrayConstructor singleton;
    return &singleton;
  }

  // 15.5.1.1 Array ( [ value ] )
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  // 15.5.2.1 new Array ( [ value ] )
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() == 1 && arguments[0]->IsNumber()) {
      Number* len = static_cast<Number*>(arguments[0]);
      if (len->data() == ToUint32(e, len)) {
        return new ArrayObject(len->data());
      } else {
        *e = *Error::RangeError(u"Invalid array length");
        return nullptr;
      }
    }
    ArrayObject* arr = new ArrayObject(arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
      JSValue* arg = arguments[i];
      arr->AddValueProperty(::es::NumberToString(i), arg, true, true, true);
    }
    return arr;
  }

  static JSValue* isArray(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0 || !vals[0]->IsObject())
      return Bool::False();
    JSObject* obj = static_cast<JSObject*>(vals[0]);
    return Bool::Wrap(obj->Class() == u"Array");
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return new String(u"function Array() { [native code] }");
  }

 private:
   ArrayConstructor() :
    JSObject(OBJ_OTHER, u"Array", true, nullptr, true, true) {}
};

// 15.4.4.4 Array.prototype.concat ( [ item1 [ , item2 [ , … ] ] ] )
JSValue* ArrayProto::concat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  std::vector<JSValue*> items = {Runtime::TopValue()};
  items.insert(items.end(), vals.begin(), vals.end());
  ArrayObject* A = new ArrayObject(0);
  size_t n = 0;
  for (auto E : items) {
    bool is_array = false;
    if (E->IsObject()) {
      JSObject* O = static_cast<JSObject*>(E);
      if (O->obj_type() == JSObject::OBJ_ARRAY) {
        is_array = true;
        size_t len = ToNumber(e, O->Get(e, u"length"));
        if (!e->IsOk()) return nullptr;
        for (size_t k = 0; k < len; k++) {  // 5.b.iii
          std::u16string P = NumberToString(k);
          if (O->HasProperty(P)) {
            JSValue* sub_element = O->Get(e, P);
            if (!e->IsOk()) return nullptr;
            A->AddValueProperty(NumberToString(n), sub_element, true, true, true);
          }
          n++;
        }
      }
    }
    if (!is_array) {
      A->AddValueProperty(NumberToString(n), E, true, true, true);
      n++;
    }
  }
  return A;
}

// 15.4.4.10 Array.prototype.slice (start, end)
JSValue* ArrayProto::slice(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSObject* O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return nullptr;
  double len = ToNumber(e, O->Get(e, u"length"));
  ArrayObject* A = new ArrayObject(0);
  if (vals.size() == 0 || vals[0]->IsUndefined()) {
    *e = *Error::TypeError(u"start of Array.prototype.slice cannot be undefined");
  }
  int relative_start = ToInteger(e, vals[0]);
  if (!e->IsOk()) return nullptr;
  int k;
  if (relative_start < 0)
    k = fmax(relative_start + len, 0);
  else
    k = fmin(relative_start, len);
  int relative_end;
  if (vals.size() < 2 || vals[1]->IsUndefined()) {
    relative_end = len;
  } else {
    relative_end = ToInteger(e, vals[1]);
    if (!e->IsOk()) return nullptr;
  }
  int final;
  int n = 0;
  if (relative_end < 0)
    final = fmax(relative_end + len, 0);
  else
    final = fmin(relative_end, len);
  while (k < final) {
    std::u16string Pk = NumberToString(k);
    if (O->HasProperty(Pk)) {
      JSValue* k_value = O->Get(e, Pk);
      if (!e->IsOk()) return nullptr;
      A->AddValueProperty(NumberToString(n), k_value, true, true, true);
    }
    k++;
    n++;
  }
  return A;
}

// 15.4.4.18 Array.prototype.forEach ( callbackfn [ , thisArg ] )
JSValue* ArrayProto::forEach(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSObject* O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return nullptr;
  size_t len = ToNumber(e, O->Get(e, u"length"));
  if (vals.size() == 0 || !vals[0]->IsCallable()) {  // 4
    *e = *Error::TypeError(u"Array.prototype.forEach called on non-callable");
    return nullptr;
  }
  JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
  JSValue* T;
  if (vals.size() < 2) {
    T = Undefined::Instance();
  } else {
    T = vals[1];
  }
  ArrayObject* A = new ArrayObject(len);
  for (size_t k = 0; k < len; k++) {
    std::u16string p_k = NumberToString(k);
    bool k_present = O->HasProperty(p_k);
    if (!e->IsOk()) return nullptr;
    if (k_present) {
      JSValue* k_value = O->Get(e, p_k);
      if (!e->IsOk()) return nullptr;
      JSValue* mapped_value = callbackfn->Call(e, T, {k_value, new Number(k), O});
      if (!e->IsOk()) return nullptr;
    }
  }
  return Undefined::Instance();
}

// 15.4.4.19 Array.prototype.map ( callbackfn [ , thisArg ] )
JSValue* ArrayProto::map(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSObject* O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return nullptr;
  size_t len = ToNumber(e, O->Get(e, u"length"));
  if (vals.size() == 0 || !vals[0]->IsCallable()) {  // 4
    *e = *Error::TypeError(u"Array.prototype.map called on non-callable");
    return nullptr;
  }
  JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
  JSValue* T;
  if (vals.size() < 2) {
    T = Undefined::Instance();
  } else {
    T = vals[1];
  }
  ArrayObject* A = new ArrayObject(len);
  for (size_t k = 0; k < len; k++) {
    std::u16string p_k = NumberToString(k);
    bool k_present = O->HasProperty(p_k);
    if (!e->IsOk()) return nullptr;
    if (k_present) {
      JSValue* k_value = O->Get(e, p_k);
      if (!e->IsOk()) return nullptr;
      JSValue* mapped_value = callbackfn->Call(e, T, {k_value, new Number(k), O});
      if (!e->IsOk()) return nullptr;
      A->AddValueProperty(p_k, mapped_value, true, true, true);
    }
  }
  return A;
}

// 15.4.4.20 Array.prototype.filter ( callbackfn [ , thisArg ] )
JSValue* ArrayProto::filter(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSObject* O = ToObject(e, Runtime::TopValue());
  if (!e->IsOk()) return nullptr;
  size_t len = ToNumber(e, O->Get(e, u"length"));
  if (vals.size() == 0 || !vals[0]->IsCallable()) {  // 4
    *e = *Error::TypeError(u"Array.prototype.filter called on non-callable");
    return nullptr;
  }
  JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
  JSValue* T;
  if (vals.size() < 2) {
    T = Undefined::Instance();
  } else {
    T = vals[1];
  }
  size_t to = 0;
  ArrayObject* A = new ArrayObject(len);
  for (size_t k = 0; k < len; k++) {
    std::u16string p_k = NumberToString(k);
    bool k_present = O->HasProperty(p_k);
    if (!e->IsOk()) return nullptr;
    if (k_present) {
      JSValue* k_value = O->Get(e, p_k);
      if (!e->IsOk()) return nullptr;
      JSValue* selected = callbackfn->Call(e, T, {k_value, new Number(k), O});
      if (!e->IsOk()) return nullptr;
      if (ToBoolean(selected)) {
        A->AddValueProperty(NumberToString(to), k_value, true, true, true);
        to++;
      }
    }
  }
  return A;
}

JSValue* ObjectConstructor::keys(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  if (vals.size() < 1 || !vals[0]->IsObject()) {
    *e = *Error::TypeError(u"Object.keys called on non-object");
    return nullptr;
  }
  JSObject* O = static_cast<JSObject*>(vals[0]);
  auto properties = O->AllEnumerableProperties();
  size_t n = properties.size();
  ArrayObject* arr_obj = new ArrayObject(n);
  for (size_t index = 0; index < n; index++) {
    arr_obj->AddValueProperty(
      NumberToString(index), new String(properties[index].first), true, true, true);
  }
  return arr_obj;
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARRAY_OBJECT