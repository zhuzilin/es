#ifndef ES_TYPES_BUILTIN_ERROR_OBJECT
#define ES_TYPES_BUILTIN_ERROR_OBJECT

#include <es/types/object.h>

namespace es {

class ErrorProto : public JSObject {
 public:
  static Handle<ErrorProto> Instance() {
    static Handle<ErrorProto> singleton = ErrorProto::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (unlikely(!val.val()->IsObject())) {
      e = Error::TypeError(u"Error.prototype.toString called with non-object value");
      return Handle<JSValue>();
    }
    Handle<JSObject> O = static_cast<Handle<JSObject>>(val);
    // TODO(zhuzilin) add name
    std::u16string name = u"Error";
    Handle<JSValue> msg = Get(e, O, String::message());
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (msg.val()->IsUndefined())
      return Undefined::Instance();
    std::u16string msg_str = ::es::ToString(e, msg).val()->data();
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    return String::New(name + u": " + msg_str);
  }

 private:
  template<flag_t flag>
  static Handle<ErrorProto> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      u"Error", true, Handle<JSValue>(), false, false, nullptr);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<ErrorProto>(jsobj);
  }
};

class ErrorObject : public JSObject {
 public:
  static Handle<ErrorObject> New(Handle<Error> e) {
    Handle<JSObject> jsobj = JSObject::New<kPtrSize>(
      u"Error", true, Handle<JSValue>(), false, false, nullptr
    );

    SET_HANDLE_VALUE(jsobj.val(), kErrorOffset, e, Error);

    jsobj.val()->SetType(OBJ_ERROR);
    Handle<ErrorObject> obj(jsobj);
    obj.val()->SetPrototype(ErrorProto::Instance());
    AddValueProperty(obj, String::message(), e.val()->value(), true, false, false);
    return obj;
  }

  Handle<Error> e() { return READ_HANDLE_VALUE(this, kErrorOffset, Error); }
  Error::ErrorType ErrorType() { return e().val()->error_type(); }
  Handle<JSValue> ErrorValue() { return e().val()->value(); }

 public:
  static constexpr size_t kErrorOffset = kJSObjectOffset;
};

class ErrorConstructor : public JSObject {
 public:
  static Handle<ErrorConstructor> Instance() {
    static Handle<ErrorConstructor> singleton = ErrorConstructor::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Error() { [native code] }");
  }

 private:
  template<flag_t flag>
  static Handle<ErrorConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      u"Error", true, Handle<JSValue>(), true, true, nullptr);

    jsobj.val()->SetType(OBJ_ERROR_CONSTRUCTOR);
    return Handle<ErrorConstructor>(jsobj);
  }
};

Handle<JSObject> Construct__ErrorConstructor(Handle<Error>& e, Handle<ErrorConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ERROR_OBJECT