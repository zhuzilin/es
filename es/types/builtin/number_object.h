#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

double ToNumber(Handle<Error>& e, Handle<JSValue> input);
double ToInteger(Handle<Error>& e, Handle<JSValue> input);
Handle<JSObject> ToObject(Handle<Error>& e, Handle<JSValue> input);
Handle<String> NumberToString(double m);

class NumberProto : public JSObject {
 public:
  static Handle<NumberProto> Instance() {
    static Handle<NumberProto> singleton = NumberProto::New(GCFlag::SINGLE);
    return singleton;
  }

  // 15.7.4.2 Number.prototype.toString ( [ radix ] )
  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsNumber() && !val.val()->IsNumberObject()) {
      e = Error::TypeError(u"Number.prototype.toString called by non-number");
      return Handle<JSValue>();
    }
    double num = ToNumber(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    int radix = 10;
    if (vals.size() > 0 && !vals[0].val()->IsUndefined()) {
      radix = ToInteger(e, vals[0]);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (radix < 2 || radix > 36) {
        e = Error::RangeError(u"Number.prototype.toString radix not in [2, 36]");
        return Handle<JSValue>();
      }
    }
    // TODO(zhuzilin) support other radix
    assert(radix == 10);
    return NumberToString(num);
  }

  static Handle<JSValue> toLocaleString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> valueOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsNumberObject()) {
      return static_cast<Handle<JSObject>>(val).val()->PrimitiveValue();
    } else if (val.val()->IsNumber()) {
      return val;
    }
    e = Error::TypeError(u"Number.prototype.valueOf called with non-number");
    return Handle<JSValue>();
  }

  static Handle<JSValue> toFixed(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toExponential(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toPrecision(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<NumberProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Number", true, Number::Zero(), false, false, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<NumberProto>(jsobj);
  }
};

class NumberObject : public JSObject {
 public:
  static Handle<NumberObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Number", true, primitive_value, false, false, nullptr, 0
    );

    jsobj.val()->SetType(OBJ_NUMBER);
    Handle<NumberObject> obj = Handle<NumberObject>(jsobj);
    obj.val()->SetPrototype(NumberProto::Instance());
    return obj;
  }
};

class NumberConstructor : public JSObject {
 public:
  static  Handle<NumberConstructor> Instance() {
    static  Handle<NumberConstructor> singleton = NumberConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Number() { [native code] }");
  }

 private:
  static Handle<NumberConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Number", true, Handle<JSValue>(), true, true, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_NUMBER_CONSTRUCTOR);
    return Handle<NumberConstructor>(jsobj);
  }
};

Handle<JSValue> Call__NumberConstructor(Handle<Error>& e, Handle<NumberConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__NumberConstructor(Handle<Error>& e, Handle<NumberConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT