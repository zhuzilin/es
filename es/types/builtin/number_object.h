#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>

namespace es {

class NumberConstructor : public JSObject {
 public:
  static  NumberConstructor* Instance() {
    static  NumberConstructor singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    return nullptr;
  }

 private:
   NumberConstructor() :
    JSObject(
      OBJ_OTHER, u"Number", false, nullptr, true, true
    ) {}
};

class NumberProto : public JSObject {
 public:
  static  NumberProto* Instance() {
    static  NumberProto singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Undefined::Instance();
  }

  static JSValue* toString(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleString(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* valueOf(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toFixed(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toExponential(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toPrecision(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   NumberProto() :
    JSObject(
      OBJ_OTHER, u"Number", true, nullptr, false, true
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

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT