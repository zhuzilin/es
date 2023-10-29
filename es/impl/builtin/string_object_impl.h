#ifndef ES_IMPL_BUILTIN_STRING_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_STRING_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

Handle<JSValue> StringProto::split(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  CheckObjectCoercible(e, val);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  std::u16string S = ToU16String(e, val);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  size_t lim = 4294967295.0;
  if (vals.size() >= 2 && !vals[1].val()->IsUndefined()) {
    lim = ToUint32(e, vals[1]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  }
  size_t s = S.size();
  size_t p = 0;
  if (lim == 0)
    return ArrayObject::New(0);
  if (vals.size() < 1 || vals[0].val()->IsUndefined()) {  // 10
    Handle<ArrayObject> A = ArrayObject::New(1);
    AddValueProperty(A, String::Zero(), String::New(S), true, true, true);
    return A;
  }
  if (vals[0].val()->IsRegExpObject()) {
    assert(false);
  }
  ASSERT(vals[0].val()->IsString());
  std::u16string R = ToU16String(e, vals[0]);
  if (s == 0) {
    regex::MatchResult z = regex::SplitMatch(S, 0, R);
    if (!z.failed) return ArrayObject::New(0);
    Handle<ArrayObject> A = ArrayObject::New(1);
    AddValueProperty(A, String::Zero(), String::New(S), true, true, true);
    return A;
  }
  size_t length_A = 0;
  size_t q = p;
  std::vector<Handle<String>> segs;
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
        segs.emplace_back(String::New(T));
        length_A++;
        if (length_A == lim) {
          Handle<ArrayObject> A = ArrayObject::New(length_A);
          for (size_t i = 0; i < segs.size(); ++i) {
            AddValueProperty(A, NumberToString(i), String::New(T), true, true, true);
          }
          return A;
        }
        p = e;
        for (size_t i = 0; i < cap.size(); i++) {  // 13.c.iii.7
          segs.emplace_back(String::New(cap[i]));
          length_A++;
          if (length_A == lim) {
            Handle<ArrayObject> A = ArrayObject::New(length_A);
            for (size_t i = 0; i < segs.size(); ++i) {
              AddValueProperty(A, NumberToString(i), String::New(T), true, true, true);
            }
            return A;
          }
        }
        q = p;
      }
    }
  }
  std::u16string T = S.substr(p);  // 14
  segs.emplace_back(String::New(T));  // 15
  length_A++;

  Handle<ArrayObject> A = ArrayObject::New(length_A);
  for (size_t i = 0; i < segs.size(); ++i) {
    AddValueProperty(A, NumberToString(i), segs[i], true, true, true);
  }
  return A;
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_STRING_OBJECT_IMPL_H