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

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<ErrorProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_ERROR, u"Error", true, Handle<JSValue>(), false, false, nullptr, 0, flag);

    return Handle<ErrorProto>(new (jsobj.val()) ErrorProto());
  }
};

class ErrorObject : public JSObject {
 public:
  static Handle<ErrorObject> New(Error* e) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_ERROR, u"Error", true, Handle<JSValue>(), false, false, nullptr, kPtrSize
    );

    SET_VALUE(jsobj.val(), kErrorOffset, e, Error*);

    Handle<ErrorObject> obj(new (jsobj.val()) ErrorObject());
    obj.val()->SetPrototype(ErrorProto::Instance());
    AddValueProperty(obj, u"message", String::New(e->message()), true, false, false);
    return obj;
  }

  Error* e() { return READ_VALUE(this, kErrorOffset, Error*); }
  Error::Type ErrorType() { return e()->type(); }
  std::u16string ErrorMessage() { return e()->message(); }

  std::string ToString() { return log::ToString(e()->message()); }

 private:
  static constexpr size_t kErrorOffset = kJSObjectOffset;
};

class ErrorConstructor : public JSObject {
 public:
  static Handle<ErrorConstructor> Instance() {
    static Handle<ErrorConstructor> singleton = ErrorConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Error() { [native code] }");
  }

 private:
  static Handle<ErrorConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_ERROR_CONSTRUCTOR, u"Error", true, Handle<JSValue>(), true, true, nullptr, 0, flag);

    return Handle<ErrorConstructor>(new (jsobj.val()) ErrorConstructor());
  }
};

Handle<JSObject> Construct__ErrorConstructor(Error* e, Handle<ErrorConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ERROR_OBJECT