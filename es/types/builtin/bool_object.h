#ifndef ES_TYPES_BUILTIN_BOOL_OBJECT
#define ES_TYPES_BUILTIN_BOOL_OBJECT

#include <es/types/object.h>

namespace es {

bool ToBoolean(JSValue*);

class BoolProto : public JSObject {
 public:
  static  BoolProto* Instance() {
    static  BoolProto singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Undefined::Instance();
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return ToBoolean(this_arg) ? String::True() : String::False();
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   BoolProto() :
    JSObject(
      OBJ_BOOL, u"Boolean", true, Bool::False(), false, true
    ) {}
};

class BoolObject : public JSObject {
 public:
  BoolObject(JSValue* primitive_value) :
    JSObject(OBJ_BOOL, u"Boolean", true, primitive_value, false, false) {
    SetPrototype(BoolProto::Instance());
  }
};

class BoolConstructor : public JSObject {
 public:
  static  BoolConstructor* Instance() {
    static  BoolConstructor singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    bool b;
    if (arguments.size() == 0)
      b = ToBoolean(Undefined::Instance());
    else
      b = ToBoolean(arguments[0]);
    return Bool::Wrap(b);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    bool b;
    if (arguments.size() == 0)
      b = ToBoolean(Undefined::Instance());
    else
      b = ToBoolean(arguments[0]);
    return new BoolObject(Bool::Wrap(b));
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return new String(u"function Bool() { [native code] }");
  }

 private:
   BoolConstructor() :
    JSObject(OBJ_OTHER, u"Boolean", true, nullptr, true, true) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_BOOL_OBJECT