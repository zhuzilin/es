#ifndef ES_TYPES_BUILTIN_BOOL_OBJECT
#define ES_TYPES_BUILTIN_BOOL_OBJECT

#include <es/types/object.h>

namespace es {

bool ToBoolean(Handle<JSValue>);

class BoolProto : public JSObject {
 public:
  static Handle<BoolProto> Instance() {
    static Handle<BoolProto> singleton = BoolProto::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return ToBoolean(Runtime::TopValue()) ? String::True() : String::False();
  }

  static Handle<JSValue> valueOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<BoolProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Boolean", true, Bool::False(), false, false, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<BoolProto>(jsobj);
  }
};

class BoolObject : public JSObject {
 public:
  static Handle<BoolObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Boolean", true, primitive_value, false, false, nullptr, 0
    );

    jsobj.val()->SetType(OBJ_BOOL);

    Handle<BoolObject> obj(jsobj);
    obj.val()->SetPrototype(BoolProto::Instance());
    return obj;
  }
};

class BoolConstructor : public JSObject {
 public:
  static Handle<BoolConstructor> Instance() {
    static Handle<BoolConstructor> singleton = BoolConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Bool() { [native code] }");
  }

 private:
  static Handle<BoolConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Boolean", true, Handle<JSValue>(), true, true, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_BOOL_CONSTRUCTOR);
    return Handle<BoolConstructor>(jsobj);
  }
};

Handle<JSValue> Call__BoolConstructor(Handle<Error>& e, Handle<BoolConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__BoolConstructor(Handle<Error>& e, Handle<BoolConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_BOOL_OBJECT