#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

double ToNumber(Error* e, Handle<JSValue> input);
double ToInteger(Error* e, Handle<JSValue> input);
Handle<JSObject> ToObject(Error* e, Handle<JSValue> input);
Handle<String> NumberToString(double m);

class NumberProto : public JSObject {
 public:
  static Handle<NumberProto> Instance() {
    static Handle<NumberProto> singleton = NumberProto::New();
    return singleton;
  }

  // 15.7.4.2 Number.prototype.toString ( [ radix ] )
  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsObject()) {
      Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
      if (obj.val()->obj_type() != JSObject::OBJ_NUMBER) {
        *e = *Error::TypeError(u"Number.prototype.toString called by non-number");
        return Handle<JSValue>();
      }
    } else if (!val.val()->IsNumber()) {
      *e = *Error::TypeError(u"Number.prototype.toString called by non-number");
      return Handle<JSValue>();
    }
    double num = ToNumber(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    int radix = 10;
    if (vals.size() > 0 && !vals[0].val()->IsUndefined()) {
      radix = ToInteger(e, vals[0]);
      if (!e->IsOk()) return Handle<JSValue>();
      if (radix < 2 || radix > 36) {
        *e = *Error::RangeError(u"Number.prototype.toString radix not in [2, 36]");
        return Handle<JSValue>();
      }
    }
    // TODO(zhuzilin) support other radix
    assert(radix == 10);
    return NumberToString(num);
  }

  static Handle<JSValue> toLocaleString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> valueOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsNumberObject()) {
      return static_cast<Handle<JSObject>>(val).val()->PrimitiveValue();
    } else if (val.val()->IsNumber()) {
      return val;
    }
    *e = *Error::TypeError(u"Number.prototype.valueOf called with non-number");
    return Handle<JSValue>();
  }

  static Handle<JSValue> toFixed(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toExponential(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toPrecision(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<NumberProto> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_NUMBER, u"Number", true, Number::Zero(), false, false, nullptr, 0);
    return Handle<NumberProto>(new (jsobj.val()) NumberProto());
  }
};

class NumberObject : public JSObject {
 public:
  static Handle<NumberObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_NUMBER, u"Number", true, primitive_value, false, false, nullptr, 0
    );
    Handle<NumberObject> obj = Handle<NumberObject>(new (jsobj.val()) NumberObject());
    obj.val()->SetPrototype(NumberProto::Instance());
    return obj;
  }
};

class NumberConstructor : public JSObject {
 public:
  static  Handle<NumberConstructor> Instance() {
    static  Handle<NumberConstructor> singleton = NumberConstructor::New();
    return singleton;
  }

  // 15.7.1.1 Number ( [ value ] )
  Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) override {
    Handle<Number> js_num;
    if (arguments.size() == 0) {
      js_num = Number::Zero();
    } else {
      double num = ToNumber(e, arguments[0]);
      if (!e->IsOk()) return Handle<JSValue>();
      js_num = Number::New(num);
    }
    return js_num;
  }

  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    Handle<Number> js_num;
    if (arguments.size() == 0) {
      js_num = Number::Zero();
    } else {
      double num = ToNumber(e, arguments[0]);
      if (!e->IsOk()) return Handle<JSValue>();
      js_num = Number::New(num);
    }
    return NumberObject::New(js_num);
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Number() { [native code] }");
  }

 private:
  static Handle<NumberConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"Number", true, Handle<JSValue>(), true, true, nullptr, 0);
    return Handle<NumberConstructor>(new (jsobj.val()) NumberConstructor());
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT