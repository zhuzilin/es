#ifndef ES_TYPES_BUILTIN_REGEX_OBJECT
#define ES_TYPES_BUILTIN_REGEX_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

JSValue ToString(JSValue& e, JSValue input);

namespace regex_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"RegExp", true, JSValue(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = regex_proto::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue exec(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue test(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue compile(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace regex_proto

namespace regex_object {

constexpr size_t kPatternOffset = js_object::kJSObjectOffset;
constexpr size_t kFlagOffset = kPatternOffset + sizeof(JSValue);
constexpr size_t kGlobalOffset = kFlagOffset + sizeof(JSValue);
constexpr size_t kIgnoreCaseOffset = kGlobalOffset + kBoolSize;
constexpr size_t kMultilineOffset = kIgnoreCaseOffset + kBoolSize;
constexpr size_t kRegExpObjectOffset = kMultilineOffset + kBoolSize;

inline JSValue New(JSValue pattern, JSValue flag) {
  JSValue jsobj = js_object::New(
    u"RegExp", true, JSValue(), false, false, nullptr,
    kRegExpObjectOffset - js_object::kJSObjectOffset
  );

  SET_JSVALUE(jsobj.handle().val(), kPatternOffset, pattern);
  SET_JSVALUE(jsobj.handle().val(), kFlagOffset, flag);
  bool global = false, ignore_case = false, multiline = false;
  for (auto c : string::data(flag)) {
    switch (c) {
      case u'g':
        global = true;
        break;
      case u'i':
        ignore_case = true;
        break;
      case u'm':
        multiline = true;
        break;
    }
  }
  SET_VALUE(jsobj.handle().val(), kGlobalOffset, global, bool);
  SET_VALUE(jsobj.handle().val(), kIgnoreCaseOffset, ignore_case, bool);
  SET_VALUE(jsobj.handle().val(), kMultilineOffset, multiline, bool);
  jsobj.SetType(OBJ_REGEXP);

  js_object::SetPrototype(jsobj, regex_proto::Instance());
  AddValueProperty(jsobj, u"source", pattern, false, false, false);
  AddValueProperty(jsobj, u"global", boolean::New(global), false, false, false);
  AddValueProperty(jsobj, u"ignoreCase", boolean::New(ignore_case), false, false, false);
  AddValueProperty(jsobj, u"multiline", boolean::New(multiline), false, false, false);
  // TODO(zhuzilin) Not sure if this should be initialized to 0.
  AddValueProperty(jsobj, u"lastIndex", number::Zero(), false, false, false);
  return jsobj;
}

inline JSValue pattern(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kPatternOffset); }
inline JSValue flag(JSValue jsval) { return GET_JSVALUE(jsval.handle().val(), kFlagOffset); }
inline bool global(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kGlobalOffset, bool); }
inline bool ignore_case(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kIgnoreCaseOffset, bool); }
inline bool multiline(JSValue jsval) { return READ_VALUE(jsval.handle().val(), kMultilineOffset, bool); }

}  // namespace regex_object


namespace regex_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"RegExp", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_REGEXP_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = regex_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function RegExp() { [native code] }");
}

}  // namespace regex_constructor

JSValue regex_proto::toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (!val.IsRegExpObject()) {
    e = error::TypeError(u"RegExp.prototype.toString called by non-regex");
    return JSValue();
  }
  return string::New(
    u"/" + string::data(regex_object::pattern(val)) + u"/" +
    (regex_object::global(val) ? u"g" : u"") +
    (regex_object::ignore_case(val) ? u"i" : u"") +
    (regex_object::multiline(val) ? u"m" : u"")
  );
}

JSValue Call__RegExpConstructor(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments = {});
JSValue Construct__RegExpConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_REGEX_OBJECT