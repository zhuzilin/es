#ifndef ES_IMPL_BUILTIN_ARRAY_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_ARRAY_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

// 15.4.4.2 Array.prototype.toString ( )
JSValue array_proto::toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue array = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue func = Get(e, array, string::New(u"join"));
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (!func.IsCallable()) {
    func = Get(e, object_proto::Instance(), string::New(u"toString"));
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  return Call(e, func, this_arg, vals);
}

// 15.4.4.4 Array.prototype.concat ( [ item1 [ , item2 [ , … ] ] ] )
JSValue array_proto::concat(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  std::vector<JSValue> items = {Runtime::TopValue()};
  items.insert(items.end(), vals.begin(), vals.end());
  size_t total_len = 0;
  for (auto E : items) {
    if (E.IsArrayObject()) {
      size_t len = ToNumber(e, Get(e, E, string::Length()));
      total_len += len;
    } else {
      total_len++;
    }
  }
  JSValue A = array_object::New(total_len);
  size_t n = 0;
  for (auto E : items) {
    if (E.IsArrayObject()) {
      size_t len = ToNumber(e, Get(e, E, string::Length()));
      if (unlikely(!error::IsOk(e))) return JSValue();
      for (size_t k = 0; k < len; k++) {  // 5.b.iii
        JSValue P = NumberToString(k);
        if (HasProperty(E, P)) {
          JSValue sub_element = Get(e, E, P);
          if (unlikely(!error::IsOk(e))) return JSValue();
          AddValueProperty(A, NumberToU16String(n), sub_element, true, true, true);
        }
        n++;
      }
    } else {
      AddValueProperty(A, NumberToU16String(n), E, true, true, true);
      n++;
    }
  }
  return A;
}

// 15.4.4.5 Array.prototype.join (separator)
JSValue array_proto::join(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue len_val = Get(e, O, string::Length());
  size_t len = number::data(len_val);

  std::u16string sep = u",";
  if (vals.size() > 0 && !vals[0].IsUndefined()) {
    sep = ToU16String(e, vals[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  if (len == 0)
    return string::Empty();
  JSValue element0 = Get(e, O, string::New(u"0"));
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string R = u"";
  if (!element0.IsUndefined() && !element0.IsNull()) {
    R = ToU16String(e, element0);
  }
  for (double k = 1; k < len; k++) {
    JSValue element = Get(e, O, NumberToString(k));
    if (unlikely(!error::IsOk(e))) return JSValue();
    std::u16string next = u"";
    if (!element.IsUndefined() && !element.IsNull()) {
      next = ToU16String(e, element);
    }
    R += sep + next;
  }
  return string::New(R);
}

// 15.4.4.6 Array.prototype.pop ( )
JSValue array_proto::pop(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t len = ToNumber(e, Get(e, O, string::Length()));
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (len == 0) {
    Put(e, O, string::Length(), number::Zero(), true);
    if (unlikely(!error::IsOk(e))) return JSValue();
    return undefined::New();
  } else {
    ASSERT(len > 0);
    JSValue indx = NumberToString(len - 1);
    JSValue element = Get(e, O, indx);
    if (unlikely(!error::IsOk(e))) return JSValue();
    Delete(e, O, indx, true);
    if (unlikely(!error::IsOk(e))) return JSValue();
    Put(e, O, string::Length(), number::New(len - 1), true);
    if (unlikely(!error::IsOk(e))) return JSValue();
    return element;
  }
}

// 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
JSValue array_proto::push(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  double n = ToNumber(e, Get(e, O, string::Length()));
  if (unlikely(!error::IsOk(e))) return JSValue();
  for (JSValue E : vals) {
    Put(e, O, NumberToString(n), E, true);
    if (unlikely(!error::IsOk(e))) return JSValue();
    n++;      
  }
  JSValue num = number::New(n);
  Put(e, O, string::Length(), num, true);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return num;
}

// 15.4.4.9 Array.prototype.shift ( )
JSValue array_proto::shift(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t len = ToNumber(e, Get(e, O, string::Length()));
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (len == 0) {
    Put(e, O, string::Length(), number::Zero(), true);
    if (unlikely(!error::IsOk(e))) return JSValue();
    return undefined::New();
  }
  JSValue first = Get(e, O, string::Zero());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t k = 1;
  while (k < len) {  // 7
    JSValue from = NumberToString(k);
    JSValue to = NumberToString(k - 1);
    bool from_present = HasProperty(O, from);
    if (from_present) {
      JSValue from_val = Get(e, O, from);
      if (unlikely(!error::IsOk(e))) return JSValue();
      Put(e, O, to, from_val, true);
      if (unlikely(!error::IsOk(e))) return JSValue();
    } else {
      Delete(e, O, to, true);
      if (unlikely(!error::IsOk(e))) return JSValue();
    }
    k++;
  }
  Delete(e, O, NumberToString(len - 1), true);
  if (unlikely(!error::IsOk(e))) return JSValue();
  Put(e, O, string::Length(), number::New(len - 1), true);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return first;
}

// 15.4.4.10 Array.prototype.slice (start, end)
JSValue array_proto::slice(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  double len = ToNumber(e, Get(e, O, string::Length()));
  if (vals.size() == 0 || vals[0].IsUndefined()) {
    e = error::TypeError(u"start of Array.prototype.slice cannot be undefined");
  }
  int relative_start = ToInteger(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int k;
  if (relative_start < 0)
    k = fmax(relative_start + len, 0);
  else
    k = fmin(relative_start, len);
  int relative_end;
  if (vals.size() < 2 || vals[1].IsUndefined()) {
    relative_end = len;
  } else {
    relative_end = ToInteger(e, vals[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  int final;
  if (relative_end < 0)
    final = fmax(relative_end + len, 0);
  else
    final = fmin(relative_end, len);
  JSValue A = array_object::New(final - k);
  int n = 0;
  while (k < final) {
    JSValue Pk = NumberToString(k);
    if (HasProperty(O, Pk)) {
      JSValue k_value = Get(e, O, Pk);
      if (unlikely(!error::IsOk(e))) return JSValue();
      AddValueProperty(A, NumberToU16String(n), k_value, true, true, true);
    }
    k++;
    n++;
  }
  return A;
}

// 15.4.4.11 Array.prototype.sort (comparefn)
JSValue array_proto::sort(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue obj = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t len = ToNumber(e, Get(e, obj, string::Length()));
  // TODO(zhuzilin) Check the implementation dependecy cases would not cause error.
  JSValue comparefn;
  if (vals.size() == 0 || vals[0].IsUndefined()) {
    comparefn = undefined::New();
  } else if (vals[0].IsCallable()) {
    comparefn = vals[0];
  } else {
    e = error::TypeError(u"comparefn of Array.prototype.sort is not callable");
    return JSValue();
  }
  std::vector<std::pair<bool, JSValue>> indices;
  for (size_t i = 0; i < len; i++) {
    JSValue istr = NumberToString(i);
    if (HasProperty(obj, istr)) {
      JSValue val = Get(e, obj, istr);
      if (unlikely(!error::IsOk(e))) return JSValue();
      indices.emplace_back(std::make_pair(true, val));
    } else {
      indices.emplace_back(std::make_pair(false, JSValue()));
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
      JSValue x = pair_x.second;
      JSValue y = pair_y.second;
      if (y.IsUndefined())
        return false;
      if (x.IsUndefined() && !y.IsUndefined())
        return true;
      if (!comparefn.IsUndefined()) {
        JSValue res = Call(e, comparefn, undefined::New(), {x, y});
        if (unlikely(!error::IsOk(e))) return false;
        return ToNumber(e, res) < 0;
      }
      // NOTE(zhuzilin) 123 will be smaller 21... which I've checked with V8.
      std::u16string xstr = ToU16String(e, x);
      std::u16string ystr = ToU16String(e, y);
      return xstr < ystr;
    });
  if (unlikely(!error::IsOk(e))) return JSValue();
  for (size_t i = 0; i < len; i++) {
    JSValue istr = NumberToString(i);
    JSValue val = indices[i].second;
    if (indices[i].first) {
      Put(e, obj, istr, val, true);
      if (unlikely(!error::IsOk(e))) return JSValue();
    } else {
      Delete(e, obj, istr, true);
      if (unlikely(!error::IsOk(e))) return JSValue();
    }
  }
  return obj;
}

// 15.4.4.18 Array.prototype.forEach ( callbackfn [ , thisArg ] )
JSValue array_proto::forEach(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t len = ToNumber(e, Get(e, O, string::Length()));
  if (vals.size() == 0 || !vals[0].IsCallable()) {  // 4
    e = error::TypeError(u"Array.prototype.forEach called on non-callable");
    return JSValue();
  }
  JSValue callbackfn = vals[0];
  JSValue T;
  if (vals.size() < 2) {
    T = undefined::New();
  } else {
    T = vals[1];
  }
  for (size_t k = 0; k < len; k++) {
    JSValue p_k = NumberToString(k);
    bool k_present = HasProperty(O, p_k);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (k_present) {
      JSValue k_value = Get(e, O, p_k);
      if (unlikely(!error::IsOk(e))) return JSValue();
      Call(e, callbackfn, T, {k_value, number::New(k), O});
      if (unlikely(!error::IsOk(e))) return JSValue();
    }
  }
  return undefined::New();
}

// 15.4.4.19 Array.prototype.map ( callbackfn [ , thisArg ] )
JSValue array_proto::map(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t len = ToNumber(e, Get(e, O, string::Length()));
  if (vals.size() == 0 || !vals[0].IsCallable()) {  // 4
    e = error::TypeError(u"Array.prototype.map called on non-callable");
    return JSValue();
  }
  JSValue callbackfn = vals[0];
  JSValue T;
  if (vals.size() < 2) {
    T = undefined::New();
  } else {
    T = vals[1];
  }
  JSValue A = array_object::New(len);
  for (size_t k = 0; k < len; k++) {
    JSValue p_k = NumberToString(k);
    bool k_present = HasProperty(O, p_k);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (k_present) {
      JSValue k_value = Get(e, O, p_k);
      if (unlikely(!error::IsOk(e))) return JSValue();
      JSValue mapped_value = Call(e, callbackfn, T, {k_value, number::New(k), O});
      if (unlikely(!error::IsOk(e))) return JSValue();
      AddValueProperty(A, string::data(p_k), mapped_value, true, true, true);
    }
  }
  return A;
}

// 15.4.4.20 Array.prototype.filter ( callbackfn [ , thisArg ] )
JSValue array_proto::filter(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue O = ToObject(e, Runtime::TopValue());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t len = ToNumber(e, Get(e, O, string::Length()));
  if (vals.size() == 0 || !vals[0].IsCallable()) {  // 4
    e = error::TypeError(u"Array.prototype.filter called on non-callable");
    return JSValue();
  }
  JSValue callbackfn = vals[0];
  JSValue T;
  if (vals.size() < 2) {
    T = undefined::New();
  } else {
    T = vals[1];
  }
  size_t to = 0;
  JSValue A = array_object::New(len);
  for (size_t k = 0; k < len; k++) {
    JSValue p_k = NumberToString(k);
    bool k_present = HasProperty(O, p_k);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (k_present) {
      JSValue k_value = Get(e, O, p_k);
      if (unlikely(!error::IsOk(e))) return JSValue();
      JSValue selected = Call(e, callbackfn, T, {k_value, number::New(k), O});
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (ToBoolean(selected)) {
        AddValueProperty(A, NumberToU16String(to), k_value, true, true, true);
        to++;
      }
    }
  }
  return A;
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_ARRAY_OBJECT_IMPL_H