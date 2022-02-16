#ifndef ES_TYPES_BUILTIN_GLOBAL_OBJECT
#define ES_TYPES_BUILTIN_GLOBAL_OBJECT

#include <es/types/object.h>

namespace es {

std::u16string ToU16String(Handle<Error>& e, Handle<JSValue> input);
double ToInt32(Handle<Error>& e, Handle<JSValue> input);
double StringToNumber(std::u16string source);
double ToNumber(Handle<Error>& e, Handle<JSValue> input);

// 15.1 The Global Object
class GlobalObject : public JSObject {
 public:
  static Handle<GlobalObject> Instance() {
    static Handle<GlobalObject> singleton(GlobalObject::New(GCFlag::SINGLE));
    return singleton;
  }

  bool direct_eval() { return READ_VALUE(this, kDirectEvalOffset, bool); }
  void SetDirectEval(bool direct_eval) {
    SET_VALUE(this, kDirectEvalOffset, direct_eval, bool);
  }

  static Handle<JSValue> eval(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> parseInt(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> parseFloat(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> isNaN(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // 15.1.2.5 isFinite (number)
  static Handle<JSValue> isFinite(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  // 15.1.3.1 decodeURI (encodedURI)
  static Handle<JSValue> decodeURI(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  // 15.1.3.2 decodeURIComponent (encodedURIComponent)
  static Handle<JSValue> decodeURIComponent(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  // 15.1.3.3 encodeURI (uri)
  static Handle<JSValue> encodeURI(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  // 15.1.3.4 encodeURIComponent (uriComponent)
  static Handle<JSValue> encodeURIComponent(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<GlobalObject> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      // 15.1 The values of the [[Prototype]] and [[Class]]
      // of the global object are implementation-dependent.
      u"Global",
      // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
      // otherwise we cannot define variable in global code, as global varaibles
      // are the property of global object.
      true, Handle<JSValue>(), false, false, nullptr, kBoolSize, flag
    );

    SET_VALUE(jsobj.val(), kDirectEvalOffset, false, bool);
    jsobj.val()->SetType(OBJ_GLOBAL);
    return Handle<GlobalObject>(jsobj);
  }

  static constexpr size_t kDirectEvalOffset = kJSObjectOffset;
};

class DirectEvalGuard {
  public:
    DirectEvalGuard() {
      GlobalObject::Instance().val()->SetDirectEval(true);
    }

    ~DirectEvalGuard() {
      GlobalObject::Instance().val()->SetDirectEval(false);
    }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_GLOBAL_OBJECT