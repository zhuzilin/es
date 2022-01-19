#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>
#include <es/execution_context.h>

namespace es {

double ToNumber(Error* e, JSValue* input);
JSObject* ToObject(Error* e, JSValue* input);

class NumberProto : public JSObject {
 public:
  static  NumberProto* Instance() {
    static  NumberProto singleton;
    return &singleton;
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (val->IsObject()) {
      JSObject* obj = static_cast<JSObject*>(val);
      if (obj->obj_type() == JSObject::OBJ_NUMBER) {
        return obj->PrimitiveValue();
      }
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
   NumberProto() :
    JSObject(
      OBJ_NUMBER, u"Number", true, Number::Zero(), false, false
    ) {}
};

class NumberObject : public JSObject {
 public:
  NumberObject(JSValue* primitive_value) :
    JSObject(
      OBJ_NUMBER,
      u"Number",
      true,  // extensible
      primitive_value,
      false,
      false
    ) {
    SetPrototype(NumberProto::Instance());
  }
};

class NumberConstructor : public JSObject {
 public:
  static  NumberConstructor* Instance() {
    static  NumberConstructor singleton;
    return &singleton;
  }

  // 15.7.1.1 Number ( [ value ] )
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    Number* js_num;
    if (arguments.size() == 0) {
      js_num = Number::Zero();
    } else {
      double num = ToNumber(e, arguments[0]);
      if (!e->IsOk()) return nullptr;
      js_num = new Number(num);
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
      js_num = new Number(num);
    }
    return new NumberObject(js_num);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return new String(u"function Number() { [native code] }");
  }

 private:
   NumberConstructor() :
    JSObject(OBJ_OTHER, u"Number", true, nullptr, true, true) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT