#ifndef ES_IMPL_BUILTIN_STRING_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_STRING_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

JSValue string_proto::split(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  std::u16string S = ToU16String(e, val);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue A = array_object::New(0);
  size_t length_A = 0;
  size_t lim = 4294967295.0;
  if (vals.size() >= 2 && !vals[1].IsUndefined()) {
    lim = ToUint32(e, vals[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  size_t s = S.size();
  size_t p = 0;
  if (lim == 0)
    return A;
  if (vals.size() < 1 || vals[0].IsUndefined()) {  // 10
    AddValueProperty(A, u"0", string::New(S), true, true, true);
    return A;
  }
  if (vals[0].IsRegExpObject()) {
    assert(false);
  }
  ASSERT(vals[0].IsString());
  std::u16string R = ToU16String(e, vals[0]);
  if (s == 0) {
    regex::MatchResult z = regex::SplitMatch(S, 0, R);
    if (!z.failed) return A;
    AddValueProperty(A, u"0", string::New(S), true, true, true);
    return A;
  }
  size_t q = p;
  while (q != s) {  // 13.
    regex::MatchResult z = regex::SplitMatch(S, q, R);  // 13.a
    if (z.failed) {  // 13.b
      q++;
    } else {  // 13.c
      size_t e = z.state.end_index;
      std::vector<std::u16string> cap = z.state.captures;
      if (e == p) {  // 13.c.ii
        q++;
      } else {  // 13.c.iii
        std::u16string T = S.substr(p, q - p);
        AddValueProperty(A, NumberToString(length_A), string::New(T), true, true, true);
        length_A++;
        if (length_A == lim)
          return A;
        p = e;
        for (size_t i = 0; i < cap.size(); i++) {  // 13.c.iii.7
          AddValueProperty(A, NumberToString(length_A), string::New(cap[i]), true, true, true);
          length_A++;
          if (length_A == lim)
            return A;
        }
        q = p;
      }
    }
  }
  std::u16string T = S.substr(p);  // 14
  AddValueProperty(A, NumberToString(length_A), string::New(T), true, true, true);  // 15
  return A;
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_STRING_OBJECT_IMPL_H