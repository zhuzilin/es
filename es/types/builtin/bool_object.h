#ifndef ES_TYPES_BUILTIN_BOOL_OBJECT
#define ES_TYPES_BUILTIN_BOOL_OBJECT

#include <es/types/object.h>

namespace es {

bool ToBoolean(JSValue*);

class BoolProto : public JSObject {
 public:
  static BoolProto* Instance() {
    static BoolProto* singleton = BoolProto::New();
    return singleton;
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return ToBoolean(this_arg) ? String::True() : String::False();
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  static BoolProto* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_BOOL, u"Boolean", true, Bool::False(), false, false, nullptr, 0);
    return new (jsobj) BoolProto();
  }
};

class BoolObject : public JSObject {
 public:
  static BoolObject* New(JSValue* primitive_value) {
    JSObject* jsobj = JSObject::New(
      OBJ_BOOL, u"Boolean", true, primitive_value, false, false, nullptr, 0
    );
    BoolObject* obj = new (jsobj) BoolObject();
    obj->SetPrototype(BoolProto::Instance());
    return obj;
  }
};

class BoolConstructor : public JSObject {
 public:
  static BoolConstructor* Instance() {
    static BoolConstructor* singleton = BoolConstructor::New();
    return singleton;
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
    return BoolObject::New(Bool::Wrap(b));
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"function Bool() { [native code] }");
  }

 private:
  static BoolConstructor* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"Boolean", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) BoolConstructor();
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_BOOL_OBJECT