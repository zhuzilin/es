#ifndef ES_TYPES_BUILTIN_ERROR_OBJECT
#define ES_TYPES_BUILTIN_ERROR_OBJECT

#include <es/types/object.h>

namespace es {

class ErrorProto : public JSObject {
 public:
  static  ErrorProto* Instance() {
    static  ErrorProto singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Undefined::Instance();
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   ErrorProto() :
    JSObject(OBJ_ERROR, u"Error", true, nullptr, false, true) {}
};

class ErrorObject : public JSObject {
 public:
  ErrorObject(Error* e) :
    JSObject(OBJ_ERROR, u"Error", true, nullptr, false, false), e_(e) {
    SetPrototype(ErrorProto::Instance());
    AddValueProperty(u"message", new String(e->message()), true, false, false);
  }

  Error::Type ErrorType() { return e_->type(); }
  std::u16string ErrorMessage() { return e_->message(); }

  std::string ToString() { return log::ToString(e_->message()); }

 private:
  Error* e_;
};

class ErrorConstructor : public JSObject {
 public:
  static  ErrorConstructor* Instance() {
    static  ErrorConstructor singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() == 0 || arguments[0]->IsUndefined())
      return new ErrorObject(Error::NativeError(::es::ToString(nullptr, Undefined::Instance())));
    std::u16string s = ::es::ToString(e, arguments[0]);
    if (!e->IsOk())
      return nullptr;
    return new ErrorObject(Error::NativeError(s));
  }

 private:
   ErrorConstructor() :
    JSObject(OBJ_OTHER, u"Error", true, nullptr, true, true) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ERROR_OBJECT