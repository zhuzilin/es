#ifndef ES_TYPES_BUILTIN_ERROR_OBJECT
#define ES_TYPES_BUILTIN_ERROR_OBJECT

#include <es/types/object.h>

namespace es {

class ErrorProto : public JSObject {
 public:
  static Handle<ErrorProto> Instance() {
    static Handle<ErrorProto> singleton = ErrorProto::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<ErrorProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Error", true, Handle<JSValue>(), false, false, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<ErrorProto>(jsobj);
  }
};

class ErrorObject : public JSObject {
 public:
  static Handle<ErrorObject> New(Handle<Error> e) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Error", true, Handle<JSValue>(), false, false, nullptr, kPtrSize
    );

    SET_HANDLE_VALUE(jsobj.val(), kErrorOffset, e, Error);

    jsobj.val()->SetType(OBJ_ERROR);
    Handle<ErrorObject> obj(jsobj);
    obj.val()->SetPrototype(ErrorProto::Instance());
    AddValueProperty(obj, u"message", e.val()->value(), true, false, false);
    return obj;
  }

  Handle<Error> e() { return READ_HANDLE_VALUE(this, kErrorOffset, Error); }
  Error::ErrorType ErrorType() { return e().val()->type(); }
  Handle<JSValue> ErrorValue() { return e().val()->value(); }

  std::string ToString() { return ErrorValue().ToString(); }

 private:
  static constexpr size_t kErrorOffset = kJSObjectOffset;
};

class ErrorConstructor : public JSObject {
 public:
  static Handle<ErrorConstructor> Instance() {
    static Handle<ErrorConstructor> singleton = ErrorConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Error() { [native code] }");
  }

 private:
  static Handle<ErrorConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      u"Error", true, Handle<JSValue>(), true, true, nullptr, 0, flag);

    jsobj.val()->SetType(OBJ_ERROR_CONSTRUCTOR);
    return Handle<ErrorConstructor>(jsobj);
  }
};

Handle<JSObject> Construct__ErrorConstructor(Handle<Error>& e, Handle<ErrorConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ERROR_OBJECT