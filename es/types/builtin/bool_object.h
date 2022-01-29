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

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return ToBoolean(Runtime::TopValue()) ? String::True() : String::False();
  }

  static Handle<JSValue> valueOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<BoolProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_BOOL, u"Boolean", true, Bool::False(), false, false, nullptr, 0, flag);

    return Handle<BoolProto>(new (jsobj.val()) BoolProto());
  }
};

class BoolObject : public JSObject {
 public:
  static Handle<BoolObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_BOOL, u"Boolean", true, primitive_value, false, false, nullptr, 0
    );

    Handle<BoolObject> obj(new (jsobj.val()) BoolObject());
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

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Bool() { [native code] }");
  }

 private:
  static Handle<BoolConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_BOOL_CONSTRUCTOR, u"Boolean", true, Handle<JSValue>(), true, true, nullptr, 0, flag);

    return Handle<BoolConstructor>(new (jsobj.val()) BoolConstructor());
  }
};

Handle<JSValue> Call__BoolConstructor(Error* e, Handle<BoolConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__BoolConstructor(Error* e, Handle<BoolConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_BOOL_OBJECT