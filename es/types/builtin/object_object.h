#ifndef ES_TYPES_BUILTIN_OBJECT_OBJECT
#define ES_TYPES_BUILTIN_OBJECT_OBJECT

#include <es/types/object.h>

namespace es {

class ObjectConstructor : public JSObject {
 public:
  static ObjectConstructor* Instance() {
    static ObjectConstructor singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    return nullptr;
  }

  // 15.2.3.2 Object.getPrototypeOf ( O )
  static JSValue* getPrototypeOf(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getOwnPropertyDescriptor(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* getOwnPropertyNames(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* create(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* defineProperty(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* defineProperties(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* seal(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* freeze(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* preventExtensions(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* isSealed(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* isFrozen(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* isExtensible(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* keys(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  ObjectConstructor() :
    JSObject(
      OBJ_OTHER, u"Object", false, nullptr, true, true
    ) {}
};

class ObjectProto : public JSObject {
 public:
  static ObjectProto* Instance() {
    static ObjectProto singleton;
    return &singleton;
  }

  static JSValue* toString(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toLocaleString(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* valueOf(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* hasOwnProperty(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* isPrototypeOf(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* propertyIsEnumerable(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  ObjectProto() :
    JSObject(
      OBJ_OTHER, u"Object", true, nullptr, false, false
    ) {}
};

class Object : public JSObject {
 public:
  Object() :
    JSObject(
      OBJ_OTHER, u"Object", true, nullptr, false, false
    ) {
    SetPrototype(ObjectProto::Instance());
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_OBJECT_OBJECT