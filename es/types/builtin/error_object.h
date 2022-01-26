#ifndef ES_TYPES_BUILTIN_ERROR_OBJECT
#define ES_TYPES_BUILTIN_ERROR_OBJECT

#include <es/types/object.h>

namespace es {

class ErrorProto : public JSObject {
 public:
  static ErrorProto* Instance() {
    static ErrorProto* singleton = ErrorProto::New();
    return singleton;
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  static ErrorProto* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_ERROR, u"Error", true, nullptr, false, false, nullptr, 0);
    return new (jsobj) ErrorProto();
  }
};

class ErrorObject : public JSObject {
 public:
  static ErrorObject* New(Error* e) {
    JSObject* jsobj = JSObject::New(
      OBJ_ERROR, u"Error", true, nullptr, false, false, nullptr, kPtrSize
    );
    SET_VALUE(jsobj, kErrorOffset, e, Error*);
    ErrorObject* obj = new (jsobj) ErrorObject();
    obj->SetPrototype(ErrorProto::Instance());
    obj->AddValueProperty(u"message", String::New(e->message()), true, false, false);
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
  static ErrorConstructor* Instance() {
    static ErrorConstructor* singleton = ErrorConstructor::New();
    return singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() == 0 || arguments[0]->IsUndefined())
      return ErrorObject::New(Error::NativeError(ToU16String(nullptr, Undefined::Instance())));
    std::u16string s = ToU16String(e, arguments[0]);
    if (!e->IsOk())
      return nullptr;
    return ErrorObject::New(Error::NativeError(s));
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"function Error() { [native code] }");
  }

 private:
  static ErrorConstructor* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"Error", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) ErrorConstructor();
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ERROR_OBJECT