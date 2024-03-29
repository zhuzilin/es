#ifndef ES_TYPES_BUILTIN_BOOL_OBJECT
#define ES_TYPES_BUILTIN_BOOL_OBJECT

#include <es/types/object.h>

namespace es {

bool ToBoolean(Handle<JSValue>);

class BoolProto : public JSObject {
 public:
  static Handle<BoolProto> Instance() {
    static Handle<BoolProto> singleton = BoolProto::New<GCFlag::SINGLE>();
    return singleton;
  }

  // 15.6.4.2 Boolean.prototype.toString ( )
  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsBool()) {
      return ToBoolean(val) ? String::True() : String::False();
    } else if (val.val()->IsBoolObject()) {
      return ToBoolean(static_cast<Handle<JSObject>>(val).val()->PrimitiveValue()) ?
        String::True() : String::False();
    } else {
      e = Error::TypeError(u"Boolean.prototype.toString called on non-boolean");
      return Handle<JSValue>();
    }
  }

  // 15.6.4.3 Boolean.prototype.valueOf ( )
  static Handle<JSValue> valueOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsBool()) {
      return val;
    } else if (val.val()->IsBoolObject()) {
      return static_cast<Handle<JSObject>>(val).val()->PrimitiveValue();
    } else {
      e = Error::TypeError(u"Boolean.prototype.valueOf called on non-boolean");
      return Handle<JSValue>();
    }
  }

 private:
  template<flag_t flag>
  static Handle<BoolProto> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_BOOL, true, Bool::False(), false, false, nullptr);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<BoolProto>(jsobj);
  }
};

class BoolObject : public JSObject {
 public:
  static Handle<BoolObject> New(Handle<JSValue> primitive_value) {
    Handle<JSObject> jsobj = JSObject::New<0>(
      CLASS_BOOL, true, primitive_value, false, false, nullptr
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
    static Handle<BoolConstructor> singleton = BoolConstructor::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Bool() { [native code] }");
  }

 private:
  template<flag_t flag>
  static Handle<BoolConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_BOOL, true, Handle<JSValue>(), true, true, nullptr);

    jsobj.val()->SetType(OBJ_BOOL_CONSTRUCTOR);
    return Handle<BoolConstructor>(jsobj);
  }
};

Handle<JSValue> Call__BoolConstructor(Handle<Error>& e, Handle<BoolConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__BoolConstructor(Handle<Error>& e, Handle<BoolConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_BOOL_OBJECT