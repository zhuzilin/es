#ifndef ES_TYPES_BUILTIN_GLOBAL_OBJECT
#define ES_TYPES_BUILTIN_GLOBAL_OBJECT

#include <es/types/object.h>

namespace es {

std::u16string ToU16String(JSValue& e, JSValue input);
double ToInt32(JSValue& e, JSValue input);
double StringToNumber(std::u16string source);
double ToNumber(JSValue& e, JSValue input);

// 15.1 The Global Object
namespace global_object {

constexpr size_t kDirectEvalOffset = js_object::kJSObjectOffset;

JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    // 15.1 The values of the [[Prototype]] and [[Class]]
    // of the global object are implementation-dependent.
    u"Global",
    // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
    // otherwise we cannot define variable in global code, as global varaibles
    // are the property of global object.
    true, JSValue(), false, false, nullptr, kBoolSize, flag
  );

  SET_VALUE(jsobj.handle().val(), kDirectEvalOffset, false, bool);
  jsobj.SetType(OBJ_GLOBAL);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton(global_object::New(GCFlag::SINGLE));
  return singleton;
}

bool direct_eval(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kDirectEvalOffset, bool); }
void SetDirectEval(JSValue jsval, bool direct_eval) {
  SET_VALUE(jsval.handle().val(), kDirectEvalOffset, direct_eval, bool);
}

inline JSValue eval(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue parseInt(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue parseFloat(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue isNaN(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

// 15.1.2.5 isFinite (number)
inline JSValue isFinite(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

// 15.1.3.1 decodeURI (encodedURI)
inline JSValue decodeURI(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

// 15.1.3.2 decodeURIComponent (encodedURIComponent)
inline JSValue decodeURIComponent(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

// 15.1.3.3 encodeURI (uri)
inline JSValue encodeURI(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

// 15.1.3.4 encodeURIComponent (uriComponent)
inline JSValue encodeURIComponent(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue escape(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue unescape(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace global_object

class DirectEvalGuard {
  public:
    DirectEvalGuard() {
      global_object::SetDirectEval(global_object::Instance(), true);
    }

    ~DirectEvalGuard() {
      global_object::SetDirectEval(global_object::Instance(), false);
    }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_GLOBAL_OBJECT