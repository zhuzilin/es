#ifndef ES_TYPES_BUILTIN_DATE_OBJECT
#define ES_TYPES_BUILTIN_DATE_OBJECT

#include <es/types/object.h>

namespace es {

class DateProto : public JSObject {
 public:
  static Handle<DateProto> Instance() {
    static Handle<DateProto> singleton = DateProto::New();
    return singleton;
  }

  Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) override {
    return Undefined::Instance();
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toDateString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toTimeString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toLocaleString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toLocaleDateString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toLocaleTimeString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> valueOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getTime(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getFullYear(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCFullYear(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getMonth(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCMonth(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getDate(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCDate(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getDay(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCDay(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getHours(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCHours(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getMinutes(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCMinutes(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getSeconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCSeconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getMilliseconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getUTCMilliseconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getTimezoneOffset(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setTime(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setMilliseconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCMilliseconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setSeconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCSeconds(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setMinutes(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCMinutes(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setHours(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCHours(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setDate(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCDate(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setMonth(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCMonth(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setFullYear(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> setUTCFullYear(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toUTCString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toISOString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toJSON(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<DateProto> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_DATE, u"Date", true, Handle<JSValue>(), false, false, nullptr, 0);
    return Handle<DateProto>(new (jsobj.val()) DateProto());
  }
};

class DateObject : public JSObject {
 public:
  static Handle<DateObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_DATE, u"Date", true, Handle<JSValue>(), false, false, nullptr, 0
    );
    Handle<DateObject> obj(new (jsobj.val()) DateObject());
    obj.val()->SetPrototype(DateProto::Instance());
    return obj;
  }

  std::string ToString() { return "Date"; }
};

class DateConstructor : public JSObject {
 public:
  static Handle<DateConstructor> Instance() {
    static Handle<DateConstructor> singleton = DateConstructor::New();
    return singleton;
  }

  Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) override {
    return Construct(e, arguments);
  }

  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    assert(false);
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Date() { [native code] }");
  }

  static Handle<JSValue> parse(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> UTC(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> now(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<DateConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"Date", true, Handle<JSValue>(), true, true, nullptr, 0);
    return Handle<DateConstructor>(new (jsobj.val()) DateConstructor());
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_DATE_OBJECT