#ifndef ES_TYPES_BUILTIN_DATE_OBJECT
#define ES_TYPES_BUILTIN_DATE_OBJECT

#include <es/types/object.h>

namespace es {

class DateProto : public JSObject {
 public:
  static  DateProto* Instance() {
  static  DateProto singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Undefined::Instance();
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toDateString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toTimeString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleDateString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleTimeString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getTime(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getFullYear(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCFullYear(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getMonth(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCMonth(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getDate(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCDate(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getDay(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCDay(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getHours(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCHours(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getMinutes(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCMinutes(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getSeconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCSeconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getMilliseconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getUTCMilliseconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getTimezoneOffset(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setTime(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setMilliseconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCMilliseconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setSeconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCSeconds(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setMinutes(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCMinutes(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setHours(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCHours(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setDate(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCDate(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setMonth(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCMonth(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setFullYear(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* setUTCFullYear(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toUTCString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toISOString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toJSON(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   DateProto() :
    JSObject(OBJ_DATE, u"Date", true, nullptr, false, true) {}
};

class DateObject : public JSObject {
 public:
  DateObject() :
    JSObject(OBJ_DATE, u"Date", true, nullptr, false, false) {
    SetPrototype(DateProto::Instance());
  }

  std::string ToString() { return "Date"; }
};

class DateConstructor : public JSObject {
 public:
  static  DateConstructor* Instance() {
  static  DateConstructor singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    assert(false);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return new String(u"function Date() { [native code] }");
  }

  static JSValue* parse(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* UTC(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* now(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   DateConstructor() :
    JSObject(OBJ_OTHER, u"Date", true, nullptr, true, true) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_DATE_OBJECT