#ifndef ES_TYPES_BUILTIN_OBJECT_OBJECT
#define ES_TYPES_BUILTIN_OBJECT_OBJECT

#include <es/types/object.h>
#include <es/types/conversion.h>

namespace es {

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

class ObjectConstructor : public JSObject {
 public:
  static ObjectConstructor* Instance() {
    static ObjectConstructor singleton;
    return &singleton;
  }

  // 15.2.1 The Object Constructor Called as a Function
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    if (arguments.size() == 0 || arguments[0]->IsNull() || arguments[0]->IsUndefined())
      return Construct(e, arguments);
    return ToObject(e, arguments[0]);
  }

  // 15.2.2 The Object Constructor
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() > 0) {  // 1
      JSValue* value = arguments[0];
      switch (value->type()) {
        case JSValue::JS_OBJECT:
          // TODO(zhuzilin) deal with host object.
          return static_cast<JSObject*>(value);
        case JSValue::JS_STRING:
        case JSValue::JS_BOOL:
        case JSValue::JS_NUMBER:
          return ToObject(e, value);
        default:
          break;
      }
    }
    assert(arguments.size() == 0 || arguments[0]->IsNull() || arguments[0]->IsUndefined());
    JSObject* obj = new Object();
    return obj;
  }

  // 15.2.3.2 Object.getPrototypeOf ( O )
  static JSValue* getPrototypeOf(Error* e, std::vector<JSValue*> vals) {
    if (vals.size() < 1 || !vals[0]->IsObject()) {
      e = Error::TypeError();
      return nullptr;
    }
    return static_cast<JSObject*>(vals[0])->Prototype();
  }

  // 15.2.3.3 Object.getOwnPropertyDescriptor ( O, P )
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

}  // namespace es

#endif  // ES_TYPES_BUILTIN_OBJECT_OBJECT