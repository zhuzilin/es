#ifndef ES_TYPES_BUILTIN_STRING_OBJECT
#define ES_TYPES_BUILTIN_STRING_OBJECT

#include <es/types/object.h>

namespace es {

std::u16string ToString(Error* e, JSValue* input);

class StringProto : public JSObject {
 public:
  static  StringProto* Instance() {
    static  StringProto singleton;
    return &singleton;
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* charAt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* charCodeAt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* concat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* indexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

    static JSValue* lastIndexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* localeCompare(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* match(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* replace(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* search(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* slice(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* split(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* substring(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLowerCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleLowerCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* trim(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   StringProto() :
    JSObject(
      OBJ_OTHER, u"String", true, String::Empty(), false, false
    ) {}
};

class StringObject : public JSObject {
 public:
  StringObject(JSValue* primitive_value) :
    JSObject(
      OBJ_STRING,
      u"String",
      true,  // extensible
      primitive_value,
      false,
      false
    ) {
    SetPrototype(StringProto::Instance());
    assert(primitive_value->IsString());
    double length = static_cast<String*>(primitive_value)->data().size();
    AddValueProperty(u"length", new Number(length), false, false, false);
  }

  JSValue* GetOwnProperty(std::u16string P) override {
    assert(false);
  }

};

class StringConstructor : public JSObject {
 public:
  static  StringConstructor* Instance() {
    static  StringConstructor singleton;
    return &singleton;
  }

  // 15.5.1.1 String ( [ value ] )
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    if (arguments.size() == 0)
      return String::Empty();
    return new String(::es::ToString(e, arguments[0]));
  }

  // 15.5.2.1 new String ( [ value ] )
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() == 0)
      return new StringObject(String::Empty());
    return new StringObject(new String(::es::ToString(e, arguments[0])));
  }

  static JSValue* fromCharCode(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   StringConstructor() :
    JSObject(
      OBJ_OTHER, u"String", true, nullptr, true, true
    ) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_STRING_OBJECT