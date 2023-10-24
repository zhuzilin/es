#ifndef ES_TYPES_BUILTIN_DATE_OBJECT
#define ES_TYPES_BUILTIN_DATE_OBJECT

#include <es/types/object.h>

namespace es {

namespace date_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Date", true, JSValue(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline JSValue& Instance() {
  static JSValue singleton = date_proto::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toDateString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toTimeString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toLocaleString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toLocaleDateString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toLocaleTimeString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue valueOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getTime(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getFullYear(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCFullYear(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getMonth(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCMonth(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getDate(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCDate(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getDay(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCDay(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getHours(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCHours(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getMinutes(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCMinutes(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getSeconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCSeconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getMilliseconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getUTCMilliseconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue getTimezoneOffset(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setTime(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setMilliseconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCMilliseconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setSeconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCSeconds(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setMinutes(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCMinutes(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setHours(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCHours(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setDate(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCDate(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setMonth(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCMonth(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setFullYear(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue setUTCFullYear(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toUTCString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toISOString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue toJSON(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace date_proto

namespace date_object {

inline JSValue New(JSValue primitive_value) {
  JSValue jsobj = js_object::New(
    u"Date", true, JSValue(), false, false, nullptr, 0
  );

  jsobj.SetType(OBJ_DATE);
  js_object::SetPrototype(jsobj, date_proto::Instance());
  return jsobj;
}

}  // namespace date_object

namespace date_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Date", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_DATE_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = date_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Date() { [native code] }");
}

inline JSValue parse(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue UTC(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue now(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace date_constructor

}  // namespace es

#endif  // ES_TYPES_BUILTIN_DATE_OBJECT