#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

double ToNumber(Error* e, JSValue* input);
double ToInteger(Error* e, JSValue* input);
JSObject* ToObject(Error* e, JSValue* input);
String* NumberToString(double m);

class NumberProto : public JSObject {
 public:
  static NumberProto* Instance() {
    static NumberProto* singleton = NumberProto::New();
    return singleton;
  }

  // 15.7.4.2 Number.prototype.toString ( [ radix ] )
  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (val->IsObject()) {
      JSObject* obj = static_cast<JSObject*>(val);
      if (obj->obj_type() != JSObject::OBJ_NUMBER) {
        *e = *Error::TypeError(u"Number.prototype.toString called by non-number");
        return nullptr;
      }
    } else if (!val->IsNumber()) {
      *e = *Error::TypeError(u"Number.prototype.toString called by non-number");
      return nullptr;
    }
    double num = ToNumber(e, val);
    if (!e->IsOk()) return nullptr;
    int radix = 10;
    if (vals.size() > 0 && !vals[0]->IsUndefined()) {
      radix = ToInteger(e, vals[0]);
      if (!e->IsOk()) return nullptr;
      if (radix < 2 || radix > 36) {
        *e = *Error::RangeError(u"Number.prototype.toString radix not in [2, 36]");
        return nullptr;
      }
    }
    // TODO(zhuzilin) support other radix
    assert(radix == 10);
    return NumberToString(num);
  }

  static JSValue* toLocaleString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (val->IsNumberObject()) {
      return static_cast<JSObject*>(val)->PrimitiveValue();
    } else if (val->IsNumber()) {
      return val;
    }
    *e = *Error::TypeError(u"Number.prototype.valueOf called with non-number");
    return nullptr;
  }

  static JSValue* toFixed(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toExponential(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toPrecision(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  static NumberProto* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_NUMBER, u"Number", true, Number::Zero(), false, false, nullptr, 0);
    return new (jsobj) NumberProto();
  }
};

class NumberObject : public JSObject {
 public:
  static NumberObject* New(JSValue* primitive_value) {
    JSObject* jsobj = JSObject::New(
      OBJ_NUMBER, u"Number", true, primitive_value, false, false, nullptr, 0
    );
    NumberObject* obj = new (jsobj) NumberObject();
    obj->SetPrototype(NumberProto::Instance());
    return obj;
  }
};

class NumberConstructor : public JSObject {
 public:
  static  NumberConstructor* Instance() {
    static  NumberConstructor* singleton = NumberConstructor::New();
    return singleton;
  }

  // 15.7.1.1 Number ( [ value ] )
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    Number* js_num;
    if (arguments.size() == 0) {
      js_num = Number::Zero();
    } else {
      double num = ToNumber(e, arguments[0]);
      if (!e->IsOk()) return nullptr;
      js_num = Number::New(num);
    }
    return js_num;
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    Number* js_num;
    if (arguments.size() == 0) {
      js_num = Number::Zero();
    } else {
      double num = ToNumber(e, arguments[0]);
      if (!e->IsOk()) return nullptr;
      js_num = Number::New(num);
    }
    return NumberObject::New(js_num);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"function Number() { [native code] }");
  }

 private:
  static NumberConstructor* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"Number", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) NumberConstructor();
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT